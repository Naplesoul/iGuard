#include <chrono>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>

#include "pose.h"
#include "camera.h"
#include "detector.h"

#define FPS 10

DEFINE_string(image_path, "../data/test1.jpeg",
    "Process an image. Read all standard formats (jpg, png, bmp, etc.).");

int test_main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Detector detector;
    detector.start();

    auto cameras = Camera::getCameras();

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

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::vector<Camera> cameras = Camera::getCameras();

    // cameras[0].init();
    // cameras[1].init();

    Detector detector;
    detector.start();

    while (true) {
        auto begin = std::chrono::system_clock::now();

        std::vector<Pose3D> poses;
        for (auto &cam : cameras) {
            cv::Mat frame = cam.getFrame();
            std::vector<Pose2D> people = detector.detect(frame);

            if (people.size() > 0) {
                poses.push_back(cam.convert3DPose(people.front()));
            }
        }

        if (poses.size() > 0) {
            Pose3D pose = poses.front();

            auto it = poses.begin();
            ++it;

            for (; it != poses.end(); ++it) {
                pose = Pose3D::combine(pose, *it);
            }
        }
        
        auto end = std::chrono::system_clock::now();
        auto next_begin = begin + std::chrono::microseconds(1000000 / FPS);

        if (end < next_begin) {
            usleep(std::chrono::duration_cast<std::chrono::microseconds>(next_begin - end).count());
        }
    }
}