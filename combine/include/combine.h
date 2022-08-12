#pragma once

#include <deque>
#include <vector>
#include <string>
#include <chrono>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <jsoncpp/json/json.h>

struct BodyMetrics
{
    int armWidth = 0;
    int legWidth = 0;
    int headWidth = 0;
    int torsoWidth = 0;
    Json::Value json;
    bool update(const Json::Value &body);
};

struct Node
{
    int x, y, z, score;
    Node(): x(0), y(0), z(0), score(0) {}
    Node(int _x, int _y, int _z, int _score)
        : x(_x), y(_y), z(_z), score(_score) {}
};

struct Skeleton
{
    bool mainExist = false;
    bool minorExist = false;
    bool leftHandExist = false;
    bool rightHandExist = false;

    std::vector<Node> bodyNodes;
    std::vector<Node> leftHandNodes;
    std::vector<Node> rightHandNodes;

    Skeleton();

    void updateMain(const Json::Value &body);
    void updateMinor(const Json::Value &body);
    void updateHands(const Json::Value &hands);
};

struct MachineState
{
    bool running = false;
    int carriageX = 0;
    int carriageZ = 0;
};

class Combine
{
private:
    int mainCameraId = 1;
    int minorCameraId = 2;
    int handCameraId = 3;

    int64_t firstFrameId = -1;
    int64_t mainFrameId = 0;
    int64_t minorFrameId = 0;
    int64_t handFrameId = 0;
    Skeleton skeleton;
    MachineState machineState;
    bool enableBodyScan = true;
    BodyMetrics bodyMetrics;

    int sockfd;
    const int addrLen = sizeof(struct sockaddr_in);
    struct sockaddr_in guiAddr;
    struct sockaddr_in simAddr;

    Json::FastWriter writer;

    int64_t avgMainDiff = 0;
    int64_t avgMinorDiff = 0;
    int64_t avgHandDiff = 0;
    std::chrono::nanoseconds frameTime;
    int64_t timeDiff(int64_t frameId);

    void send();
    void sendToGuiClient();
    void sendToSimClient();
    void mainCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload);
    void minorCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload);
    void handCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload);

public:
    Combine(const Json::Value &config);
    void recv(const struct sockaddr_in &clientAddr, const Json::Value &payload);
};