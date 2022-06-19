#include <signal.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <chrono>
#include <jsoncpp/json/json.h>
#include <nuitrack/Nuitrack.h>

#include "udpsender.h"

using namespace tdv::nuitrack;

UDPSender *sender = nullptr;

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

    sender->sendPoseToServer(userSkeletons[0]);
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

    sender = new UDPSender(config["serverIp"].asCString(), config["serverPort"].asInt());
    int fps = config["fps"].asInt();

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
    while (!finished)
    {
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
            delete sender;
            execv(argv[0], argv);
        }
        catch (const Exception& e)
        {
            std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
            errorCode = EXIT_FAILURE;
        }

        auto end = std::chrono::system_clock::now();
        auto next_begin = begin + std::chrono::microseconds(1000000 / fps);

        std::chrono::nanoseconds process_time(0);
        process_time += (end - begin);
        std::cout << "Process Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "us\n\n";

        if (end < next_begin) {
            usleep(std::chrono::duration_cast<std::chrono::microseconds>(next_begin - end).count());
        }
    }

    // Release Nuitrack
    try
    {
        Nuitrack::release();
        delete sender;
        sender = nullptr;
    }
    catch (const Exception& e)
    {
        std::cerr << "Nuitrack release failed (ExceptionType: " << e.type() << ")" << std::endl;
        errorCode = EXIT_FAILURE;
    }

    return errorCode;
}