#pragma once

#include <opencv2/opencv.hpp>
#include <openpose/headers.hpp>

struct KeyPoint
{
    float x, y, score;
};

class Pose25
{
private:
    KeyPoint keyPoints[25];

public:

    enum BodyPart
    {
        HEAD = 0, NECK,
        SHOULDER_R, ELBOW_R, HAND_R,
        SHOULDER_L, ELBOW_L, HAND_L,
        HIP,
        HIP_R, KNEE_R, ANKLE_R,
        HIP_L, KNEE_L, ANKLE_L,
        EYE_R, EYE_L,
        EAR_R, EAR_L,
        BIG_TOE_L, LITTLE_TOE_L, HEEL_L,
        BIG_TOE_R, LITTLE_TOE_R, HEEL_R
    };

    Pose25(const op::Array<float> &poseKeypoints, int person);
    static void convert(std::vector<Pose25> &people,
                        const std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> &datumsPtr);
    KeyPoint &operator[](BodyPart bp);
};

class Detector
{
private:
    op::Wrapper opWrapper;
    std::vector<Pose25> people;
    std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> datumsPtr;

public:
    Detector();

    void start();
    void stop();

    std::vector<Pose25> &detect(const cv::Mat &cvImageToProcess);
    cv::Mat getProcessedMat();
};