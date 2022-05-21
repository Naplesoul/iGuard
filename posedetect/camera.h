#pragma once

#include <opencv2/opencv.hpp>

#include "pose.h"

class Camera
{
private:
    float x, y, z;

public:
    Camera(float x, float y, float z);

    cv::Mat getFrame();
    KeyPoint3D convert3D(KeyPoint2D p);
};