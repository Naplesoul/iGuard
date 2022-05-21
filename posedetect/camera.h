#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include "pose.h"

class Camera
{
private:
    float x, y, z;

    rs2::config cfg;
    rs2::context ctx;
    rs2::pipeline pipe;
    rs2::colorizer color_map;

    rs2::depth_frame dframe;

public:
    Camera(rs2::config &cfg, rs2::context &ctx);

    static std::vector<Camera> getCameras();

    void init(float x, float y, float z);
    void start();
    cv::Mat getFrame();
    KeyPoint3D convert3D(KeyPoint2D p);
};