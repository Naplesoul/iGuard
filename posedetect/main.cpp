#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>

#include "detector.h"

DEFINE_string(image_path, "../data/test1.jpeg",
    "Process an image. Read all standard formats (jpg, png, bmp, etc.).");

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Detector detector;
    detector.start();

    const cv::Mat cvImageToProcess = cv::imread(FLAGS_image_path);
    for (int i = 0; i < 20; ++i) {
        const auto opTimer = op::getTimerInit();
        std::vector<Pose2D> people = detector.detect(cvImageToProcess);
        std::cout << "people count: " << people.size() << std::endl;
        op::printTime(opTimer, "detection time: ", " seconds.", op::Priority::High);
    }
    cv::imwrite("./result.png", detector.getProcessedMat());
    detector.stop();
}