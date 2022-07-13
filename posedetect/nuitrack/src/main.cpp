#include <signal.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/Core>
#include <jsoncpp/json/json.h>
#include <nuitrack/Nuitrack.h>

#include "utils.h"
#include "client.h"

using namespace tdv::nuitrack;

int fps = 10;
uint64_t frameId = 0;
DetectClient *client = nullptr;
std::chrono::nanoseconds frameTime(0);
std::chrono::system_clock::time_point firstFrameTime = stringToDateTime("2022-07-06 00::00::00");

void signalHandler(int signal)
{
    Nuitrack::release();
    delete client;
    exit(0);
}

uint64_t waitUntilNextFrame()
{
    auto now = std::chrono::system_clock::now();
    std::chrono::nanoseconds totalTime(now - firstFrameTime);
    uint64_t frameId = std::ceil(double(totalTime.count()) / double(frameTime.count()));
    std::this_thread::sleep_for(firstFrameTime + frameId * frameTime - now);
    return frameId;
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
    printf("Usage: pose_detect [path/to/camera_config.json] [path/to/nuitrack.config]\n");

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
    frameTime = std::chrono::nanoseconds(1000000000 / fps);

    Eigen::Matrix4f M_inv = convertMatrix(config);
    std::cout << "M_inv:\n" << M_inv << "\n";

    std::string serverIp = config["server_ip"].asString();
    int serverPort = config["server_port"].asInt();
    int cameraId = config["cameraId"].asInt();

    client = new DetectClient(serverIp, serverPort, cameraId, M_inv);
    SkeletonTracker::Ptr skeletonTracker = nullptr;

    // start Nuitrack
    try
    {
        Nuitrack::init(configPath);
        std::string serial = config["serial"].asString();
        std::string deviceName = config["device_name"].asString();
        auto devices = Nuitrack::getDeviceList();
        for (auto &device : devices) {
            std::string devSerial = device->getInfo(tdv::nuitrack::device::DeviceInfoType::SERIAL_NUMBER);
            if (devSerial == serial) {
                printf("Found Camera Id = %d, Serial = %s, Name = %s\n", cameraId, serial.c_str(), deviceName.c_str());
                Nuitrack::setDevice(device);
                break;
            }
        }
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
        while (true) {
            frameId = waitUntilNextFrame();
            auto begin = std::chrono::system_clock::now();
            
            // Wait for new skeleton tracking data
            Nuitrack::waitUpdate(skeletonTracker);

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
}