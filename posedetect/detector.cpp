#include <gflags/gflags.h>
#include <opencv2/opencv.hpp>
#include <openpose/headers.hpp>

#include "detector.h"

DEFINE_int32(fps_max, -1, "Max fps can run at, -1 means no limit.");

DEFINE_bool(disable_multi_thread, false, "Whether to disable multi-thread.");
DEFINE_bool(blend_original_frame, true, "Whether to blend original frame.");

DEFINE_string(net_resolution, "-1x368", "Input resolution of the net, less can be faster and less accuracy, must be multiples of 16.");
DEFINE_string(model_path, "../models",  "Path to caffe models directory.");
DEFINE_string(pose_model, "BODY_25",    "Model to be used. `BODY_25` (fastest for CUDA version, most accurate, and includes"
                                        " foot keypoints), `COCO` (18 keypoints), `MPI` (15 keypoints, least accurate model but"
                                        " fastest on CPU), `MPI_4_layers` (15 keypoints, even faster but less accurate).");

Detector::Detector(): opWrapper(op::ThreadManagerMode::Asynchronous)
{
    op::WrapperStructPose wrapperStructPose;
    wrapperStructPose.netInputSize = op::flagsToPoint(op::String(FLAGS_net_resolution), "-1x368");
    wrapperStructPose.poseModel = op::flagsToPoseModel(op::String(FLAGS_pose_model));
    wrapperStructPose.blendOriginalFrame = FLAGS_blend_original_frame;
    wrapperStructPose.modelFolder = op::String(FLAGS_model_path);
    wrapperStructPose.fpsMax = FLAGS_fps_max;

    opWrapper.configure(wrapperStructPose);
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

std::vector<Pose2D> &
Detector::detect(const cv::Mat &cvImageToProcess)
{
    const op::Matrix imageToProcess = OP_CV2OPCONSTMAT(cvImageToProcess);
    datumsPtr = opWrapper.emplaceAndPop(imageToProcess);
    people.clear();
    Pose2D::convert(people, datumsPtr);
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