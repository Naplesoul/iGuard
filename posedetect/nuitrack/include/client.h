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

class DetectClient
{
private:
    int sockfd;
    int addrLen;
    struct sockaddr_in serverAddr;

    int cameraId;
    Eigen::Matrix4f M_inv;
    Json::FastWriter writer;

    void sendToServer(const char* buf, int len);

public:
    void sendEmpty(uint64_t frameId);
    void send(uint64_t frameId, const tdv::nuitrack::Skeleton &newSkeleton);

    DetectClient(const std::string &addr, uint16_t port, int cameraId, Eigen::Matrix4f M_inv);
    ~DetectClient();
};