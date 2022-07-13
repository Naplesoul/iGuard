#include "combine.h"

#include <memory>
#include <string.h>
#include <arpa/inet.h>

void Node::diff(const Node &ancher)
{
    x -= ancher.x;
    y -= ancher.y;
    z -= ancher.z;
}

Skeleton::Skeleton()
{
    bodyNodes.resize(19);
    leftHandNodes.resize(21);
    rightHandNodes.resize(21);
}

void Skeleton::updateMain(const Json::Value &body)
{

}

void Skeleton::updateMinor(const Json::Value &body)
{

}

void Skeleton::updateHands(const Json::Value &hands)
{
    
}

Combine::Combine(const char *guiIp, const char *simIp, int guiPort, int simPort)
{
    memset(&guiAddr, 0, sizeof(guiAddr));
    memset(&simAddr, 0, sizeof(simAddr));
    
    guiAddr.sin_family = AF_INET;
    guiAddr.sin_addr.s_addr = inet_addr(guiIp);
    guiAddr.sin_port = htons(guiPort);

    simAddr.sin_family = AF_INET;
    simAddr.sin_addr.s_addr = inet_addr(simIp);
    simAddr.sin_port = htons(simPort);
}

void Combine::recv(const Json::Value &payload)
{
    int cameraId = payload["camera_id"].asInt();
    if (cameraId == mainCameraId) {
        mainCameraRecv(payload);
    } else if (cameraId == minorCameraId) {
        minorCameraRecv(payload);
    } else if (cameraId == handCameraId) {
        handCameraRecv(payload);
    } else {
        printf("Unknown camera id: %d\n", cameraId);
    }
}

void Combine::sendToGuiClient()
{
    
}

void Combine::sendToSimClient()
{

}

void Combine::mainCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    if (frameId > mainFrameId) {
        
    }
}

void Combine::minorCameraRecv(const Json::Value &payload)
{

}

void Combine::handCameraRecv(const Json::Value &payload)
{

}