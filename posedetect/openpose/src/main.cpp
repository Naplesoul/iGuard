#include <chrono>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <gflags/gflags.h>
#include <jsoncpp/json/json.h>

#include "pose.h"
#include "camera.h"
#include "detector.h"
#include "udpsender.h"

#define FPS 2

DEFINE_string(image_path, "../data/test1.jpeg",
    "Process an image. Read all standard formats (jpg, png, bmp, etc.).");
DEFINE_string(server_ip, "59.78.8.125", "ipv4 address of the server");
DEFINE_int32(server_port, 50001, "udp port of the server");

CameraParams cam_425 {
    .serial = "819612071399",
    .w_ratio = (581*64) / (752*48),
    .w = 640, .h = 480,
    .x = 0, .y = 1., .z = 0,
    .r_x_x = 1, .r_x_y = 0, .r_x_z = 0,
    .r_y_x = 0, .r_y_y = 1, .r_y_z = 0,
    .r_z_x = 0, .r_z_y = 0, .r_z_z = 1
};

std::vector<CameraParams> params({ cam_425 });

int test_main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Detector detector;
    detector.start();

    auto cameras = Camera::getCameras(params);

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
    UDPSender sender(FLAGS_server_ip.c_str(), FLAGS_server_port);

    std::vector<Camera> cameras = Camera::getCameras(params);

    if (cameras.size() <= 0) {
        std::cout << "No initialized cameras\n";
        exit(-1);
    }
    
    for (auto &cam : cameras) {
        cam.start();
    }

    Detector detector;
    detector.start();

    while (true) {
        auto begin = std::chrono::system_clock::now();

        std::vector<Pose3D> poses;
        for (auto &cam : cameras) {
            cv::Mat &frame = cam.getFrame();
            std::vector<Pose2D> people = detector.detect(frame);
            cv::imwrite("./result.png", detector.getProcessedMat());

            if (people.size() > 0) {
                Pose3D pose = cam.convert3DPose(people.front());
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

            printf("\n 3D:\n");
            for (int i = 0; i < BODY_PART_CNT; ++i) {
                printf("{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f, \"score\":%.2f},\n", pose[BodyPart(i)].x, pose[BodyPart(i)].y, pose[BodyPart(i)].z, pose[BodyPart(i)].score);
            }
            sender.sendPoseToServer(pose);
        } else {
            printf("no person found\n");
        }
         
        auto end = std::chrono::system_clock::now();
        auto next_begin = begin + std::chrono::microseconds(1000000 / FPS);

        std::chrono::nanoseconds process_time(0);
        process_time += (end - begin);
        std::cout << "Process Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n\n";

        if (end < next_begin) {
            usleep(std::chrono::duration_cast<std::chrono::microseconds>(next_begin - end).count());
        }
    }
}