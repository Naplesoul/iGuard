#include <vector>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>

#include "camera.h"

Camera::Camera(rs2::config &_cfg, rs2::context &_ctx):
    x(0), y(0), z(0),
    cfg(_cfg), ctx(_ctx), pipe(_ctx), dframe(nullptr)
{

}

std::vector<Camera> Camera::getCameras()
{
    rs2::context ctx;
    std::vector<std::string> serials;
    std::vector<Camera> cameras;

    for (auto &&dev : ctx.query_devices()) {
        serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
    }
    
    for (auto &&serial : serials) {
        rs2::config cfg;
        cfg.enable_device(serial);
        cameras.emplace_back(cfg, ctx);
    }
}

void Camera::init(float _x, float _y, float _z)
{
    x = _x;
    y = _y;
    z = _z;
}

void Camera::start()
{
    pipe.start(cfg);
}

cv::Mat Camera::getFrame()
{
    rs2::frameset data = pipe.wait_for_frames();
    dframe = data.get_depth_frame();
    rs2::frame frame = dframe.apply_filter(color_map);

    const int w = frame.as<rs2::video_frame>().get_width();
    const int h = frame.as<rs2::video_frame>().get_height();

    // Create OpenCV matrix of size (w,h) from the colorized depth data
    cv::Mat image(cv::Size(w, h), CV_8UC3, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
    return image;
}

KeyPoint3D Camera::convert3D(KeyPoint2D p)
{
    float depth = dframe.get_distance(p.x, p.y);

    KeyPoint3D p3d;
    // TODO: calculate real-world position of p

    return p3d;
}