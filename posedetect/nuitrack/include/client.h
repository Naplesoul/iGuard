#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <nuitrack/Nuitrack.h>
#include <eigen3/Eigen/Core>
#include <jsoncpp/json/json.h>

struct Point
{
    float x, y, z, score;
    Point(float x = 0, float y = 0, float z = 0, float score = 0):
        x(x), y(y), z(z), score(score) {}
};

struct Skeleton
{
    Point joints[25];
};

class DetectClient
{
private:
    int sockfd;
    int addr_len;
    struct sockaddr_in serveraddr;

    float smooth;
    float smoothRest;
    Eigen::Matrix4f M_inv;
    Json::FastWriter writer;
    Skeleton skeleton;

    void sendToServer(const char* buf, int len);
    void sendPoseToServer();

public:
    void update(const tdv::nuitrack::Skeleton &newSkeleton);

    DetectClient(const char* addr, uint16_t port, Eigen::Matrix4f M_inv, float smooth = 0.3);
    ~DetectClient();
};