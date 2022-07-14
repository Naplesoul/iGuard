#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <WinSock2.h>
#include <sys/types.h>

#include <nuitrack/Nuitrack.h>
#include <eigen3/Eigen/Core>
#include <json.h>

struct Point
{
    float x, y, z, score;
    Point(float x = 0, float y = 0, float z = 0, float score = 0):
        x(x), y(y), z(z), score(score) {}
};

struct PureSkeleton
{
    Point joints[25];
};

class DetectClient
{
private:
 //   SOCKET send_socket;
 //   int addr_len;
 //   SOCKADDR_IN serveraddr;

    uint64_t frameId;
    int cameraId;
    float smooth;
    float smoothRest;
    Eigen::Matrix4f M_inv;
    Json::FastWriter writer;
    PureSkeleton skeleton;

    void sendToServer(const char* buf, int len);
    void sendPoseToServer();

public:
    void update(uint64_t frameId, const tdv::nuitrack::Skeleton &newSkeleton);

    DetectClient(const char* addr, uint16_t port, int cameraId, Eigen::Matrix4f M_inv, float smooth = 0.3);
    ~DetectClient();
};