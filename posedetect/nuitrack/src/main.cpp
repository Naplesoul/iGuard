#include <signal.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include <jsoncpp/json/json.h>
#include <nuitrack/Nuitrack.h>

#include "utils.h"
#include "client.h"

using namespace tdv::nuitrack;

bool interrupted = false;
UDPClient *colorFrameClient = nullptr;
SkeletonClient *skeletonClient = nullptr;

ColorSensor::Ptr colorSensor = nullptr;
UserTracker::Ptr userTracker = nullptr;
SkeletonTracker::Ptr skeletonTracker = nullptr;

int fps = 10;
uint64_t frameId = 0;
std::mutex mtx;
bool enableBodyScan = false;
std::chrono::nanoseconds offset(0);
std::chrono::nanoseconds frameTime(0);
const std::chrono::system_clock::time_point firstFrameTime = stringToDateTime("2022-07-15 00::00::00");

void signalHandler(int signal)
{
    if (interrupted) exit(0);
    interrupted = true;
}

void feedback(int64_t _offset, bool body_scan)
{
    printf("Feedback: offset %ld, body_scan %d\n", _offset, body_scan);
    mtx.lock();
    offset += std::chrono::nanoseconds(_offset * 1000000);
    if (enableBodyScan && !body_scan) {
        enableBodyScan = false;
    }
    mtx.unlock();
}

uint64_t waitUntilNextFrame()
{
    auto now = std::chrono::system_clock::now();
    std::chrono::nanoseconds totalTime(now - firstFrameTime);
    uint64_t newFrameId = std::ceil(double(totalTime.count()) / double(frameTime.count()));
    if (newFrameId <= frameId) {
        newFrameId = frameId + 1;
    }
    mtx.lock();
    auto sleepTime = firstFrameTime + newFrameId * frameTime - now + offset;
    mtx.unlock();
    if (sleepTime.count() > 1000000) {
        std::this_thread::sleep_for(sleepTime);
    }
    return newFrameId;
}

void onColorUpdate(RGBFrame::Ptr frame)
{
    printf("Color Frame Update\n");
    if (frameId % 5 != 0) return;

    const tdv::nuitrack::Color3 *colorPtr = frame->getData();
    int w = frame->getCols();
    int h = frame->getRows();

    cv::Mat bgrFrame(h, w, CV_8UC3);

    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            const tdv::nuitrack::Color3 *color = colorPtr + row * w + col;
            cv::Vec3b pixel;
            pixel[0] = color->blue;
            pixel[1] = color->green;
            pixel[2] = color->red;
            bgrFrame.at<cv::Vec3b>(row, col) = pixel;
        }
    }
    // cv::imwrite("frame.png", bgrFrame);
    cv::resize(bgrFrame, bgrFrame, cv::Size(800, 450));
    int quality = 40; //压缩比率0～100
    std::vector<uint8_t> imageData;
    std::vector<int> compress_params;
    compress_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compress_params.push_back(quality);
    cv::imencode(".jpg", bgrFrame, imageData, compress_params);
    colorFrameClient->sendToServer(imageData.data(), imageData.size());
}

// @Param widthOrient: 0 for horizontal, 1 for vertical
float measureWidth(const Joint &joint, int widthOrient, float scale)
{
    UserFrame::Ptr userFrame = userTracker->getUserFrame();
    int w = userFrame->getCols();
    int h = userFrame->getRows();

    int col = joint.proj.x * w;
    int row = joint.proj.y * h;

    if (widthOrient == 0) {
        const uint16_t *rowPtr = userFrame->getData() + row * w ;
        uint16_t label = *(rowPtr + col);
        int left = col - 1;
        for (; left >= 0; --left) {
            if (*(rowPtr + left) != label) {
                break;
            }
        }

        int right = col + 1;
        for (; right < w; ++right) {
            if (*(rowPtr + right) != label) {
                break;
            }
        }

        float len = right - left;
        return len * scale / w;

    } else if (widthOrient == 1) {
        const uint16_t *labelPtr = userFrame->getData();
        uint16_t label = *(labelPtr + row * w + col);
        int up = row - 1;
        for (; up >= 0; --up) {
            if (*(labelPtr + up * w + col) != label) {
                break;
            }
        }

        int down = row + 1;
        for (; down < h; ++down) {
            if (*(labelPtr + down * w + col) != label) {
                break;
            }
        }

        float len = down - up;
        return len * scale / h;
    }
    return 0;
}

BodyMetrics scanBody(const Skeleton &skeleton)
{
    float verticalScale = abs((skeleton.joints[JOINT_LEFT_COLLAR].real.y - skeleton.joints[JOINT_TORSO].real.y)
        / (skeleton.joints[JOINT_LEFT_COLLAR].proj.y - skeleton.joints[JOINT_TORSO].proj.y));
    float horizontalScale = abs((skeleton.joints[JOINT_LEFT_SHOULDER].real.x - skeleton.joints[JOINT_RIGHT_SHOULDER].real.x)
        / (skeleton.joints[JOINT_LEFT_SHOULDER].proj.x - skeleton.joints[JOINT_RIGHT_SHOULDER].proj.x));

    int armWidth = (measureWidth(skeleton.joints[JOINT_RIGHT_ELBOW], 1, verticalScale)
        + measureWidth(skeleton.joints[JOINT_LEFT_ELBOW], 1, verticalScale)) / 2;
    int headWidth = measureWidth(skeleton.joints[JOINT_HEAD], 0, horizontalScale);
    int torsoWidth = measureWidth(skeleton.joints[JOINT_TORSO], 0, horizontalScale);

    horizontalScale = abs((skeleton.joints[JOINT_LEFT_HIP].real.x - skeleton.joints[JOINT_RIGHT_HIP].real.x)
        / (skeleton.joints[JOINT_LEFT_HIP].proj.x - skeleton.joints[JOINT_RIGHT_HIP].proj.x));
    int legWidth = (measureWidth(skeleton.joints[JOINT_RIGHT_KNEE], 0, horizontalScale)
        + measureWidth(skeleton.joints[JOINT_LEFT_KNEE], 0, horizontalScale)) / 2;

    BodyMetrics results {
        .armWidth = armWidth,
        .legWidth = legWidth,
        .headWidth = headWidth,
        .torsoWidth = torsoWidth
    };

    // UserFrame::Ptr userFrame = userTracker->getUserFrame();
    // const int MAX_LABELS = 8;
	// static uint8_t colors[MAX_LABELS][3] =
	// {
	//     {0, 0, 0},
	//     {0, 255, 0},
	//     {0, 0, 255},
	//     {255, 255, 0},
	    
	//     {0, 255, 255},
	//     {255, 0, 255},
	//     {127, 255, 0},
	//     {255, 255, 255}
	// };

    // const uint16_t *labelPtr = userFrame->getData();
    // int w = userFrame->getCols();
    // int h = userFrame->getRows();

    // cv::Mat bgrFrame(h, w, CV_8UC3);
    // for (int i = 0; i < h; ++i) {
    //     for (int j = 0; j < w; ++j) {
    //         const uint16_t label = *(labelPtr + i * w + j);
    //         cv::Vec3b pixel;
    //         pixel[0] = colors[label & 7][0];
    //         pixel[1] = colors[label & 7][1];
    //         pixel[2] = colors[label & 7][2];
    //         bgrFrame.at<cv::Vec3b>(i, j) = pixel;
    //     }
    // }

    // cv::Vec3b pixel;
    // pixel[0] = colors[2][0];
    // pixel[1] = colors[2][1];
    // pixel[2] = colors[2][2];
    // int x = skeleton.joints[JOINT_HEAD].proj.x * w;
    // int y = skeleton.joints[JOINT_HEAD].proj.y * h;
    // bgrFrame.at<cv::Vec3b>(y, x) = pixel;
    // cv::imwrite("frame.png", bgrFrame);

    return results;
}

// Callback for the hand data update event
void onSkeletonUpdate(SkeletonData::Ptr skeletonData)
{
    printf("Skeleton Tracker Update\n");
    if (!skeletonData)
    {
        // No skeleton data
        skeletonClient->sendEmpty(frameId);
        return;
    }

    auto userSkeletons = skeletonData->getSkeletons();
    if (userSkeletons.empty())
    {
        // No user skeletons
        skeletonClient->sendEmpty(frameId);
        return;
    }

    mtx.lock();
    bool scanBodyEnabled = enableBodyScan;
    mtx.unlock();

    if (scanBodyEnabled && frameId % 5 == 0) {
        skeletonClient->sendSkeletonAndMetrics(frameId, userSkeletons[0], scanBody(userSkeletons[0]));
    } else {
        skeletonClient->sendSkeleton(frameId, userSkeletons[0]);
    }
}

int main(int argc, char* argv[])
{
    printf("Usage: pose [path/to/camera_config.json] [path/to/nuitrack.config]\n");

    if (argc < 2) {
        std::cerr << "missing camera config json\n";
        exit(-1);
    }

    signal(SIGINT, &signalHandler);

    std::string configPath = "";
    if (argc > 2) {
        configPath = argv[2];
    }

    Json::Value config;
    Json::Reader reader;
    std::fstream file(argv[1]);
    reader.parse(file, config);
    file.close();

    fps = config["fps"].asInt();
    frameTime = std::chrono::nanoseconds(1000000000 / fps);

    Eigen::Matrix4f M_inv = convertMatrix(config);
    std::cout << "M_inv:\n" << M_inv << "\n";

    std::string serverIp = config["server_ip"].asString();
    int serverPort = config["server_port"].asInt();
    int cameraId = config["camera_id"].asInt();

    if (!config["color_server_ip"].empty()) {
        colorFrameClient = new UDPClient(config["color_server_ip"].asString(), config["color_server_port"].asInt());
    }

    skeletonClient = new SkeletonClient(serverIp, serverPort, cameraId, feedback, M_inv);

    // start Nuitrack
    try
    {
        Nuitrack::init(configPath);
        std::string serial = config["serial"].asString();
        std::string deviceName = config["device_name"].asString();
        bool foundDevice = false;
        auto devices = Nuitrack::getDeviceList();
        for (auto &device : devices) {
            std::string devSerial = device->getInfo(tdv::nuitrack::device::DeviceInfoType::SERIAL_NUMBER);
            if (devSerial == serial) {
                foundDevice = true;
                printf("Found Camera Id = %d, Serial = %s, Name = %s\n", cameraId, serial.c_str(), deviceName.c_str());
                Nuitrack::setDevice(device);
                break;
            }
        }

        if (!foundDevice) {
            printf("Cannot found Camera Id = %d, Serial = %s, Name = %s\n", cameraId, serial.c_str(), deviceName.c_str());
            Nuitrack::release();
            delete skeletonClient;
            if (colorFrameClient) {
                delete colorFrameClient;
                colorFrameClient = nullptr;
            }
            exit(-1);
        }

        if (colorFrameClient) {
            colorSensor = ColorSensor::create();
            colorSensor->connectOnNewFrame(onColorUpdate);
        }

        if (!config["scan_body"].empty() && config["scan_body"].asBool()) {
            enableBodyScan = true;
            userTracker = UserTracker::create();
        }

        // Create SkeletonTracker module, other required modules will be created automatically
        skeletonTracker = SkeletonTracker::create();

        // Connect onHandUpdate callback to receive hand tracking data
        skeletonTracker->connectOnUpdate(onSkeletonUpdate);

        Nuitrack::run();
    }
    catch (const Exception& e)
    {
        std::cerr << "Can not start Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    int errorCode = EXIT_SUCCESS;
    try
    {
        while (!interrupted) {
            frameId = waitUntilNextFrame();
            printf("FrameID = %ld\n", frameId);
            auto begin = std::chrono::system_clock::now();
            
            // Wait for new skeleton tracking data
            Nuitrack::waitUpdate(skeletonTracker);

            auto end = std::chrono::system_clock::now();
            std::chrono::nanoseconds process_time(end - begin);
            printf("Process Time: %ldus\n\n\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());

            mtx.lock();
            bool scanBodyEnabled = enableBodyScan;
            mtx.unlock();
            if (userTracker != nullptr && !scanBodyEnabled) {
                userTracker = nullptr;
            }
        }
    }
    catch (LicenseNotAcquiredException& e)
    {
        std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
        delete skeletonClient;
        if (colorFrameClient) {
            delete colorFrameClient;
            colorFrameClient = nullptr;
        }
        Nuitrack::release();
        execv(argv[0], argv);
    }
    catch (const Exception& e)
    {
        std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
        errorCode = EXIT_FAILURE;
    }

    delete skeletonClient;
    if (colorFrameClient) {
        delete colorFrameClient;
        colorFrameClient = nullptr;
    }
    Nuitrack::release();
    exit(errorCode);
}