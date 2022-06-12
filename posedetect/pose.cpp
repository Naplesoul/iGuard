#include <opencv2/opencv.hpp>
#include <openpose/headers.hpp>

#include "pose.h"

Pose2D::Pose2D(const op::Array<float> &poseKeypoints, int person)
{
    int partCnt = poseKeypoints.getSize(1);
    for (auto bodyPart = 0; bodyPart < partCnt; bodyPart++) {
        KeyPoint2D *kp = &(keyPoints[bodyPart]);
        kp->x = poseKeypoints[{person, bodyPart, 0}];
        kp->y = poseKeypoints[{person, bodyPart, 1}];
        kp->score = poseKeypoints[{person, bodyPart, 2}];
    }
}

KeyPoint2D &Pose2D::operator[](BodyPart bp)
{
    return keyPoints[bp];
}

void Pose2D::convert(std::vector<Pose2D> &people,
                     const std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>>& datumsPtr)
{
    try {
        if (datumsPtr != nullptr && !datumsPtr->empty()) {
            const auto& poseKeypoints = datumsPtr->at(0)->poseKeypoints;
            int peopleCnt = poseKeypoints.getSize(0);
            for (auto person = 0; person < peopleCnt; person++) {
                people.emplace_back(poseKeypoints, person);
            }
        }
        else {
            op::opLog("Nullptr or empty datumsPtr found.", op::Priority::High);
        }
    }
    catch (const std::exception& e) {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    }
}

Pose3D::Pose3D()
{

}

KeyPoint3D &Pose3D::operator[](BodyPart bp)
{
    return keyPoints[bp];
}

Pose3D Pose3D::combine(Pose3D &pose1, Pose3D &pose2)
{
    // TODO: develop an algorithm
    return pose1;
}