#include <vector>
#include <iostream>
#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include "camera.h"

Camera::Camera(rs2::config &_cfg, rs2::context &_ctx):
    x(0), y(0), z(0),
    cfg(_cfg), ctx(_ctx), pipe(_ctx), dframe(nullptr), align(rs2_stream::RS2_STREAM_COLOR)
{

}

std::vector<Camera> Camera::getCameras()
{
    rs2::context ctx;
    std::vector<std::string> serials;
    std::vector<Camera> cameras;

    for (auto &&dev : ctx.query_devices()) {
        serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    }
    
    for (auto &&serial : serials) {
        std::cout << "Found camera, serial: " << serial << std::endl;
        rs2::config cfg;
        cfg.enable_device(serial);
        cameras.emplace_back(cfg, ctx);
    }

    return cameras;
}

void Camera::init(float _w_ratio, int _w, int _h,
                  float _x, float _y, float _z,
                  float r_x_x, float r_x_y, float r_x_z,
                  float r_y_x, float r_y_y, float r_y_z,
                  float r_z_x, float r_z_y, float r_z_z)
{
    w = _w;
    h = _h;
    cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGR8, 30);
    cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, 30);

    w_ratio = _w_ratio;
    x = _x;
    y = _y;
    z = _z;

    Eigen::Matrix4f T_inv = Eigen::Matrix4f::Identity();
    T_inv(0, 3) = x;
    T_inv(1, 3) = y;
    T_inv(2, 3) = z;

    dir_x = Eigen::Vector3f(r_x_x, r_x_y, r_x_z);
    dir_y = Eigen::Vector3f(r_y_x, r_y_y, r_y_z);
    dir_z = Eigen::Vector3f(r_z_x, r_z_y, r_z_z);

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

    M_inv = T_inv * R4_inv;

    std::cout << M_inv;
}

void Camera::start()
{
    pipe.start(cfg);
}

cv::Mat Camera::getFrame()
{
    rs2::frameset data = pipe.wait_for_frames();
    data = align.process(data);
    dframe = data.get_depth_frame();
    rs2::frame frame = data.get_color_frame();
    // rs2::frame frame = dframe.apply_filter(color_map);

    // Create OpenCV matrix of size (w,h) from the colorized depth data
    cv::Mat image(cv::Size(w, h), CV_8UC3, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
    return image;
}

KeyPoint3D Camera::convert3D(KeyPoint2D p)
{
    int x = int(p.x);
    int y = int(p.y);

    float depth = dframe.get_distance(x, y);
    // printf("depth(%d, %d): %f\n", int(p.x), int(p.y), depth);

    Eigen::Vector3f cam_pos(p.x - w / 2, h / 2 - p.y, 0);
    cam_pos = cam_pos * (w_ratio / w);
    cam_pos(2) = 1;
    cam_pos.normalize();
    cam_pos = cam_pos * depth;

    Eigen::Vector4f cam_pos_4(cam_pos(0), cam_pos(1), cam_pos(2), 1);
    Eigen::Vector4f real_pos = M_inv * cam_pos_4;
    // std::cout << "\n\n\ncam_pos_4:\n" << cam_pos_4 << "\nreal_pos\n" << real_pos;

    return KeyPoint3D(real_pos(0), real_pos(1), real_pos(2), p.score);
    // return KeyPoint3D(cam_pos(0), cam_pos(1), cam_pos(2), depth);
}

Pose3D Camera::convert3DPose(Pose2D &pose)
{
    Pose3D pose_3d;
    for (int i = 0; i < BODY_PART_CNT; ++i) {
        pose_3d[BodyPart(i)] = convert3D(pose[BodyPart(i)]);
    }
    return pose_3d;
}