#include <chrono>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>

#include "pose.h"
#include "camera.h"
#include "detector.h"

#define FPS 2

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

    return 0;
}

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::vector<Camera> cameras = Camera::getCameras();

    cameras[0].init(
        3, 640, 480,
        0, 1, 0,
        1, 0, 0,
        0, 1, 0,
        0, 0, 1);
    // cameras[1].init();

    Detector detector;
    detector.start();
    cameras[0].start();

    while (true) {
        auto begin = std::chrono::system_clock::now();

        std::vector<Pose3D> poses;
        for (auto &cam : cameras) {
            cv::Mat frame = cam.getFrame();
            std::vector<Pose2D> people = detector.detect(frame);
            // if (people.size() != 1)
            cv::imwrite("./result.png", detector.getProcessedMat());
            // printf("%d\n", people.size());

            if (people.size() > 0) {
                printf("\n\n\n 2D:\n");
                for (int i = 0; i < BODY_PART_CNT; ++i) {
                    printf("[%.2f, %.2f, %.2f]\n", people.front()[BodyPart(i)].x, people.front()[BodyPart(i)].y, people.front()[BodyPart(i)].score);
                }
                printf("\n 3D:\n");
                Pose3D pose = cam.convert3DPose(people.front());
                for (int i = 0; i < BODY_PART_CNT; ++i) {
                    printf("{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f, \"score\":%.2f},\n", pose[BodyPart(i)].x, pose[BodyPart(i)].y, pose[BodyPart(i)].z, pose[BodyPart(i)].score);
                }
                poses.push_back(pose);
            }
        }

        if (poses.size() > 0) {
            Pose3D pose = poses.front();

            auto it = poses.begin();
            ++it;

            for (; it != poses.end(); ++it) {
                pose = Pose3D::combine(pose, *it);
            }

            // printf("\n\nPerson0:\n");
            // for (int i = 0; i < BODY_PART_CNT; ++i) {
            //     printf("[%.2f, %.2f, %.2f, %.2f]\n", pose[BodyPart(i)].x, pose[BodyPart(i)].y, pose[BodyPart(i)].z, pose[BodyPart(i)].score);
            // }
        } else {
            // printf("no person found\n");
        }
         
        auto end = std::chrono::system_clock::now();
        auto next_begin = begin + std::chrono::microseconds(1000000 / FPS);

        if (end < next_begin) {
            usleep(std::chrono::duration_cast<std::chrono::microseconds>(next_begin - end).count());
        }
    }
}