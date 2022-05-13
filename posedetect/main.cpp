#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>

#define OPENPOSE_FLAGS_DISABLE_POSE
#define OPENPOSE_FLAGS_DISABLE_DISPLAY
#include <openpose/flags.hpp>

#include "detector.h"

DEFINE_string(image_path, "./data/test1.jpeg",
    "Process an image. Read all standard formats (jpg, png, bmp, etc.).");

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Detector detector;
    detector.start(FLAGS_disable_multi_thread);

    const cv::Mat cvImageToProcess = cv::imread(FLAGS_image_path);
    for (;;) {
        const auto opTimer = op::getTimerInit();
        std::vector<Pose25> people = detector.detect(cvImageToProcess);
        std::cout << "people count: " << people.size() << std::endl;
        op::printTime(opTimer, "detection time: ", " seconds.", op::Priority::High);
    }
    detector.stop();
}