#include <nuitrack/Nuitrack.h>

#include <signal.h>
#include <iomanip>
#include <iostream>

using namespace tdv::nuitrack;

void showHelpInfo()
{
	std::cout << "Usage: nuitrack_console_sample [path/to/nuitrack.config]" << std::endl;
}

// Callback for the hand data update event
void onHandUpdate(SkeletonData::Ptr handData)
{
    if (!handData)
    {
        // No hand data
        std::cout << "No hand data" << std::endl;
        return;
    }

    auto userHands = handData->getSkeletons();
    if (userHands.empty())
    {
        // No user hands
        return;
    }

    auto rightHand = userHands[0].joints[JOINT_HEAD];
    // if (!rightHand)
    // {
    //     // No right hand
    //     std::cout << "Right hand of the first user is not found" << std::endl;
    //     return;
    // }

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Head position: "
                 "x = " << rightHand.real.x << ", "
                 "y = " << rightHand.real.y << ", "
                 "z = " << rightHand.real.z << std::endl;
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
    
    // Create HandTracker module, other required modules will be
    // created automatically
    auto skeletonTracker = SkeletonTracker::create();

    // Connect onHandUpdate callback to receive hand tracking data
    skeletonTracker->connectOnUpdate(onHandUpdate);

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
            // Wait for new hand tracking data
            Nuitrack::waitUpdate(skeletonTracker);
        }
        catch (LicenseNotAcquiredException& e)
        {
            std::cerr << "LicenseNotAcquired exception (ExceptionType: " << e.type() << ")" << std::endl;
            errorCode = EXIT_FAILURE;
            break;
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