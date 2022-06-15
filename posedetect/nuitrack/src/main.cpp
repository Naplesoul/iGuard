#include <nuitrack/Nuitrack.h>

#include <signal.h>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include "udpsender.h"

using namespace tdv::nuitrack;

UDPSender sender("59.78.8.125", 50001);

void showHelpInfo()
{
	std::cout << "Usage: nuitrack_console_sample [path/to/nuitrack.config]" << std::endl;
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

    sender.sendPoseToServer(userSkeletons[0]);
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

    signal(SIGINT, &signalHandler);

    std::string configPath = "";
    if (argc > 1)
        configPath = argv[1];

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
            execv("./pose_detect", argv);
        }
        catch (const Exception& e)
        {
            std::cerr << "Nuitrack update failed (ExceptionType: " << e.type() << ")" << std::endl;
            errorCode = EXIT_FAILURE;
        }
    }

    // Release Nuitrack
    try
    {
        Nuitrack::release();
    }
    catch (const Exception& e)
    {
        std::cerr << "Nuitrack release failed (ExceptionType: " << e.type() << ")" << std::endl;
        errorCode = EXIT_FAILURE;
    }

    return errorCode;
}