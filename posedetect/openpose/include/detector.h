#pragma once

#include <opencv2/opencv.hpp>
#include <openpose/headers.hpp>

#include "pose.h"

class Detector
{
private:
    op::Wrapper opWrapper;
    std::vector<Pose2D> people;
    std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> datumsPtr;

public:
    Detector();

    void start();
    void stop();

    std::vector<Pose2D> &detect(const cv::Mat &cvImageToProcess);
    cv::Mat getProcessedMat();
};