#pragma once

#include <vector>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include "pose.h"

struct CameraParams
{
    std::string serial;
    float w_ratio;
    int w, h;
    float x, y, z;
    float r_x_x, r_x_y, r_x_z;
    float r_y_x, r_y_y, r_y_z;
    float r_z_x, r_z_y, r_z_z;
};

class Camera
{
private:
    float x, y, z;
    int w, h;

    // assume a rectangle fills the picture when observed 1m away
    // its width will be w_ratio, measured in meters
    float w_ratio;

    Eigen::Vector3f dir_x, dir_y, dir_z;
    Eigen::Matrix4f M_inv;

    rs2::config cfg;
    rs2::context ctx;
    std::string serial;

    rs2::pipeline pipe;
    rs2::colorizer color_map;
    rs2::align align;

    rs2::depth_frame dframe;
    rs2::frame cframe;
    cv::Mat image;

public:
    Camera(rs2::config &cfg, rs2::context &ctx, const std::string &serial);

    static std::vector<Camera> getCameras(const std::vector<CameraParams> &params);

    void init(const CameraParams &params);
    void start();
    cv::Mat &getFrame();
    KeyPoint3D convert3D(KeyPoint2D p);
    Pose3D convert3DPose(Pose2D &pose);
};