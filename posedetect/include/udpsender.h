#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "pose.h"

class UDPSender
{
private:
    int sockfd;
    int addr_len;
    struct sockaddr_in serveraddr;

public:
    void sendToServer(const char* buf, int len);
    void sendPoseToServer(const Pose3D &pose3d);

    UDPSender(const char* addr, uint16_t port);
    ~UDPSender();
};

