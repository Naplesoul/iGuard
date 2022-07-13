#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/Core>
#include <jsoncpp/json/json.h>

std::chrono::system_clock::time_point stringToDateTime(const std::string &s)
{
    std::tm timeDate = {};
    std::istringstream ss(s);
    ss >> std::get_time(&timeDate, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(mktime(&timeDate));
}

Eigen::Matrix4f convertMatrix(const Json::Value &config)
{
    float x = config["camera_position"][0].asFloat();
    float y = config["camera_position"][1].asFloat();
    float z = config["camera_position"][2].asFloat();

    Eigen::Matrix4f T_inv = Eigen::Matrix4f::Identity();
    T_inv(0, 3) = x;
    T_inv(1, 3) = y;
    T_inv(2, 3) = z;

    Eigen::Vector3f dir_x(config["camera_direction_x"][0].asFloat(), config["camera_direction_x"][1].asFloat(), config["camera_direction_x"][2].asFloat());
    Eigen::Vector3f dir_y(config["camera_direction_y"][0].asFloat(), config["camera_direction_y"][1].asFloat(), config["camera_direction_y"][2].asFloat());
    Eigen::Vector3f dir_z(config["camera_direction_z"][0].asFloat(), config["camera_direction_z"][1].asFloat(), config["camera_direction_z"][2].asFloat());

    dir_x.normalize();
    dir_y.normalize();
    dir_z.normalize();

    Eigen::Matrix3f R3;
    R3.row(0) = dir_x;
    R3.row(1) = dir_y;
    R3.row(2) = dir_z;

    Eigen::Matrix3f R3_inv = R3.inverse();
    Eigen::Matrix4f R4_inv = Eigen::Matrix4f::Identity();
    R4_inv.block(0, 0, 3, 3) = R3_inv.block(0, 0, 3, 3);

    Eigen::Matrix4f M_inv = T_inv * R4_inv;

    return M_inv;
}