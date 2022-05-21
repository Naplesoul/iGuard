#pragma once

#include <opencv2/opencv.hpp>
#include <openpose/headers.hpp>

struct KeyPoint2D
{
    float x, y;
    float score;

    KeyPoint2D(): x(0), y(0), score(0) {}
};

struct KeyPoint3D
{
    float x, y, z;
    float score;

    KeyPoint3D(): x(0), y(0), z(0), score(0) {}
};

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

class Pose2D
{
private:
    KeyPoint2D keyPoints[25];

public:

    Pose2D(const op::Array<float> &poseKeypoints, int person);

    KeyPoint2D &operator[](BodyPart bp);
    static void convert(std::vector<Pose2D> &people,
                        const std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>> &datumsPtr);
};

class Pose3D
{
private:
    KeyPoint3D keyPoints[25];

public:

    Pose3D();

    KeyPoint3D &operator[](BodyPart bp);

    static Pose3D combine(Pose3D &pose1, Pose3D &pose2);
};