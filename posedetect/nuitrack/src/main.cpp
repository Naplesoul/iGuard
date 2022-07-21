#include <signal.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <unistd.h>
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

#define ENABLE_COLOR 0

using namespace tdv::nuitrack;

bool interrupted = false;
DetectClient *client = nullptr;

int fps = 10;
uint64_t frameId = 0;
std::mutex mtx;
std::chrono::nanoseconds offset(0);
std::chrono::nanoseconds frameTime(0);
const std::chrono::system_clock::time_point firstFrameTime = stringToDateTime("2022-07-15 00::00::00");

void signalHandler(int signal)
{
    if (interrupted) exit(0);
    interrupted = true;
}

void updateOffset(int64_t _offset)
{
    printf("update offset %ld\n", _offset);
    mtx.lock();
    offset += std::chrono::nanoseconds(_offset * 1000000);
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

void onFrameUpdate(RGBFrame::Ptr frame)
{
    const tdv::nuitrack::Color3 *colorPtr = frame->getData();

    int w = frame->getCols();
    int h = frame->getRows();

    cv::Mat bgrFrame(h, w, CV_8UC3);

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            const tdv::nuitrack::Color3 *color = colorPtr + i * w + j;
            cv::Vec3b pixel;
            pixel[0] = color->blue;
            pixel[1] = color->green;
            pixel[2] = color->red;
            bgrFrame.at<cv::Vec3b>(i, j) = pixel;
        }
    }
    cv::resize(bgrFrame, bgrFrame, cv::Size(480, 270));
    cv::imwrite("frame.png", bgrFrame);
}

// Callback for the hand data update event
void onSkeletonUpdate(SkeletonData::Ptr skeletonData)
{
    if (!skeletonData)
    {
        // No skeleton data
        client->sendEmpty(frameId);
        return;
    }

    auto userSkeletons = skeletonData->getSkeletons();
    if (userSkeletons.empty())
    {
        // No user skeletons
        client->sendEmpty(frameId);
        return;
    }

    client->send(frameId, userSkeletons[0]);
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

    client = new DetectClient(serverIp, serverPort, cameraId, updateOffset, M_inv);
#if ENABLE_COLOR
    ColorSensor::Ptr colorSensor = nullptr;
#endif
    SkeletonTracker::Ptr skeletonTracker = nullptr;

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
            delete client;
            exit(-1);
        }
#if ENABLE_COLOR
        colorSensor = ColorSensor::create();
        colorSensor->connectOnNewFrame(onFrameUpdate);
#endif
        // Create SkeletonTracker module, other required modules will be
        // created automatically
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
            auto begin = std::chrono::system_clock::now();
            
            // Wait for new skeleton tracking data
            Nuitrack::waitUpdate(skeletonTracker);
#if ENABLE_COLOR
            Nuitrack::waitUpdate(colorSensor);
#endif

            auto end = std::chrono::system_clock::now();
            std::chrono::nanoseconds process_time(end - begin);
            printf("Process Time: %ldus\n\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());
        }
    }
    catch (LicenseNotAcquiredException& e)
    {
        std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
        delete client;
        Nuitrack::release();
        execv(argv[0], argv);
    }
    catch (const Exception& e)
    {
        std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
        errorCode = EXIT_FAILURE;
    }

    delete client;
    Nuitrack::release();
    exit(errorCode);
}