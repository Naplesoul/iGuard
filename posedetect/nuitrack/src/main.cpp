#include <signal.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <thread>
#include "json.h"
#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/Core>
#include <nuitrack/Nuitrack.h>

#include "client.h"

using namespace tdv::nuitrack;

DetectClient *client = nullptr;
uint64_t frameId = 0;
int fps = 15;

std::chrono::system_clock::time_point stringToDateTime(const std::string &s)
{
    std::tm timeDate = {};
    std::istringstream ss(s);
    ss >> std::get_time(&timeDate, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(mktime(&timeDate));
}

std::chrono::system_clock::time_point firstFrameTime = stringToDateTime("2022-06-21 00:00::00");

uint64_t waitUntilNextFrame()
{
    auto now = std::chrono::system_clock::now();
    std::chrono::nanoseconds frameTime(1000000000 / fps);
    std::chrono::nanoseconds totalTime(now - firstFrameTime);
    uint64_t frameId = std::ceil(double(totalTime.count()) / double(frameTime.count()));
    std::this_thread::sleep_for(firstFrameTime + frameId * frameTime - now);
    return frameId;
}

void showHelpInfo()
{
	std::cout << "Usage: pose_detect [path/to/camera_config.json] [path/to/nuitrack.config]" << std::endl;
}

// Callback for the hand data update event
void onSkeletonUpdate(SkeletonData::Ptr skeletonData)
{
    if (!skeletonData)
    {
        // No skeleton data
        std::cout << "No skeleton data" << std::endl;
        return;
    }

    auto userSkeletons = skeletonData->getSkeletons();
    if (userSkeletons.empty())
    {
        // No user skeletons
        std::cout << "No user skeletons" << std::endl;
        return;
    }

    client->update(frameId, userSkeletons[0]);
}

bool finished;
void signalHandler(int signal)
{
    if (signal == SIGINT)
        finished = true;
}

int main(int argc, char* argv[])
{
    showHelpInfo();

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

    fps = config["fps"].asInt();

    float x = config["cameraPosition"][0].asFloat();
    float y = config["cameraPosition"][1].asFloat();
    float z = config["cameraPosition"][2].asFloat();

    Eigen::Matrix4f T_inv = Eigen::Matrix4f::Identity();
    T_inv(0, 3) = x;
    T_inv(1, 3) = y;
    T_inv(2, 3) = z;

    Eigen::Vector3f dir_x(config["cameraDirectionX"][0].asFloat(), config["cameraDirectionX"][1].asFloat(), config["cameraDirectionX"][2].asFloat());
    Eigen::Vector3f dir_y(config["cameraDirectionY"][0].asFloat(), config["cameraDirectionY"][1].asFloat(), config["cameraDirectionY"][2].asFloat());
    Eigen::Vector3f dir_z(config["cameraDirectionZ"][0].asFloat(), config["cameraDirectionZ"][1].asFloat(), config["cameraDirectionZ"][2].asFloat());

    dir_x.normalize();
    dir_y.normalize();
    dir_z.normalize();

    Eigen::Matrix3f R3;
    R3.row(0) = dir_x;
    R3.row(1) = dir_y;
    R3.row(2) = dir_z;

    Eigen::Matrix3f R3_inv = R3.inverse();
    Eigen::Matrix4f R4_inv = Eigen::Matrix4f::Identity();
    R4_inv.block(0, 0, 3, 3) = R3_inv.block(0, 0, 3, 3);

    Eigen::Matrix4f M_inv = T_inv * R4_inv;

    std::cout << "M_inv:\n" << M_inv << "\n";

    client = new DetectClient(config["serverIp"].asCString(), config["serverPort"].asInt(),
                              config["cameraId"].asInt(), M_inv, config["smooth"].asFloat());

    // Initialize Nuitrack
    try
    {
        Nuitrack::init(configPath);
    }
    catch (const Exception& e)
    {
        std::cerr << "Can not initialize Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    std::string serial = config["serial"].asString();
    auto devices = Nuitrack::getDeviceList();
    for (auto &device : devices) {
        std::string devSerial = device->getInfo(tdv::nuitrack::device::DeviceInfoType::SERIAL_NUMBER);
        if (devSerial == serial) {
            std::cout << "Found device!\n";
            Nuitrack::setDevice(device);
            break;
        }
    }
    
    // Create SkeletonTracker module, other required modules will be
    // created automatically
    auto skeletonTracker = SkeletonTracker::create();

    // Connect onHandUpdate callback to receive hand tracking data
    skeletonTracker->connectOnUpdate(onSkeletonUpdate);

    // Start Nuitrack
    try
    {
        Nuitrack::run();
    }
    catch (const Exception& e)
    {
        std::cerr << "Can not start Nuitrack (ExceptionType: " << e.type() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    int errorCode = EXIT_SUCCESS;
    auto start = std::chrono::system_clock::time_point();
    while (!finished)
    {
        frameId = waitUntilNextFrame();
        auto begin = std::chrono::system_clock::now();

        try
        {
            // Wait for new skeleton tracking data
            Nuitrack::waitUpdate(skeletonTracker);
        }
        catch (LicenseNotAcquiredException& e)
        {
            std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
            errorCode = EXIT_FAILURE;
            Nuitrack::release();
            execv(argv[0], argv);
        }
        catch (const Exception& e)
        {
            std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
            errorCode = EXIT_FAILURE;
        }

        auto end = std::chrono::system_clock::now();

        std::chrono::nanoseconds process_time(end - begin);
        std::cout << "Process Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "us\n\n";
    }

    // Release Nuitrack
    try
    {
        Nuitrack::release();
        delete client;
        client = nullptr;
    }
    catch (const Exception& e)
    {
        std::cerr << "Nuitrack release failed (ExceptionType: " << e.type() << ")" << std::endl;
        errorCode = EXIT_FAILURE;
    }

    return errorCode;
}