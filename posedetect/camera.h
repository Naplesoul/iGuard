#pragma once

#include <vector>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include "pose.h"

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
    rs2::pipeline pipe;
    rs2::colorizer color_map;

    rs2::depth_frame dframe;

public:
    Camera(rs2::config &cfg, rs2::context &ctx);

    static std::vector<Camera> getCameras();

    void init(float _w_ratio, float _x, float _y, float _z,
              float r_x_x, float r_x_y, float r_x_z,
              float r_y_x, float r_y_y, float r_y_z,
              float r_z_x, float r_z_y, float r_z_z);
    void start();
    cv::Mat getFrame();
    KeyPoint3D convert3D(KeyPoint2D p);
    Pose3D convert3DPose(Pose2D &pose);
};