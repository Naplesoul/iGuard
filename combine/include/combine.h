#pragma once

#include <deque>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <jsoncpp/json/json.h>

struct Node
{
    int x, y, z, score;
    Node(): x(0), y(0), z(0), score(0) {}
    Node(int _x, int _y, int _z, int _score)
        : x(_x), y(_y), z(_z), score(_score) {}

    void diff(const Node &ancher);
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

class Combine
{
private:
    float smooth;
    int mainCameraId = 1;
    int minorCameraId = 2;
    int handCameraId = 3;

    struct sockaddr_in guiAddr;
    struct sockaddr_in simAddr;

    int64_t mainFrameId = 0;
    int64_t minorFrameId = 0;
    int64_t handFrameId = 0;
    Skeleton skeleton;

    void sendToGuiClient();
    void sendToSimClient();

    void mainCameraRecv(const Json::Value &payload);
    void minorCameraRecv(const Json::Value &payload);
    void handCameraRecv(const Json::Value &payload);

public:
    Combine(const Json::Value &config);
    void recv(const Json::Value &payload);
};