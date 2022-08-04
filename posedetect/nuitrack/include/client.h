#pragma once

#include <thread>
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

struct BodyMetrics
{
    int armWidth = 0;
    int legWidth = 0;
    int headWidth = 0;
    int torsoWidth = 0;
};

class UDPClient
{
protected:
    int sockfd;
    int addrLen;
    struct sockaddr_in serverAddr;

public:
    UDPClient(const std::string &addr, uint16_t port);
    ~UDPClient();
    void sendToServer(const void *buf, int len);
};

class SkeletonClient: public UDPClient
{
private:
    int cameraId;
    Eigen::Matrix4f M_inv;
    Json::Reader reader;
    Json::FastWriter writer;

    void (*feedback)(int64_t, bool);
    std::thread feedbackThread;
    void pollFeedback();

public:
    void sendEmpty(uint64_t frameId);
    void sendSkeleton(uint64_t frameId, const tdv::nuitrack::Skeleton &skeleton);
    void sendSkeletonAndMetrics(uint64_t frameId, const tdv::nuitrack::Skeleton &skeleton, const BodyMetrics &bodyMetrics);

    SkeletonClient(const std::string &addr, uint16_t port, int cameraId,
                 void feedback(int64_t, bool), Eigen::Matrix4f M_inv);
    ~SkeletonClient();
};