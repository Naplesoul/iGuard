#define OPENPOSE_FLAGS_DISABLE_POSE
// #define OPENPOSE_FLAGS_DISABLE_DISPLAY
#include <openpose/flags.hpp>

#include "detector.h"

KeyPoint &Pose25::operator[](BodyPart bp)
{
    return keyPoints[bp];
}

Pose25::Pose25(const op::Array<float> &poseKeypoints, int person)
{
    int partCnt = poseKeypoints.getSize(1);
    for (auto bodyPart = 0; bodyPart < partCnt; bodyPart++) {
        KeyPoint *kp = &(keyPoints[bodyPart]);
        kp->x = poseKeypoints[{person, bodyPart, 0}];
        kp->y = poseKeypoints[{person, bodyPart, 1}];
        kp->score = poseKeypoints[{person, bodyPart, 2}];
    }
}

void Pose25::convert(std::vector<Pose25> &people,
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



Detector::Detector(): opWrapper(op::ThreadManagerMode::Asynchronous)
{

}

void Detector::start()
{
    op::opLog("Starting OpenPose...", op::Priority::High);

    if (FLAGS_disable_multi_thread)
        opWrapper.disableMultiThreading();

    op::opLog("Starting thread(s)...", op::Priority::High);
    opWrapper.start();
}

void Detector::stop()
{
    op::opLog("Stopping OpenPose...", op::Priority::High);
    opWrapper.stop();
}

std::vector<Pose25> &
Detector::detect(const cv::Mat &cvImageToProcess)
{
    const op::Matrix imageToProcess = OP_CV2OPCONSTMAT(cvImageToProcess);
    datumsPtr = opWrapper.emplaceAndPop(imageToProcess);
    people.clear();
    Pose25::convert(people, datumsPtr);
    return people;
}

cv::Mat Detector::getProcessedMat()
{
    try {
        if (datumsPtr != nullptr && !datumsPtr->empty()) {
            return OP_OP2CVCONSTMAT(datumsPtr->at(0)->cvOutputData);
        }
        else {
            op::opLog("Nullptr or empty datumsPtr found.", op::Priority::High);
        }
    }
    catch (const std::exception& e) {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    }
    return cv::Mat();
}