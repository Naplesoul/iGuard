#include "combine.h"

#include <memory>
#include <iostream>
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

void Skeleton::updateMain(const Json::Value &bodyMsg)
{
    if (bodyMsg["body_nodes"].empty()) {
        mainExist = false;
        return;
    }

    mainExist = true;
    const Json::Value &body = bodyMsg["body_nodes"];
    Node newAnchor = Node(body[2][0].asInt(), body[2][1].asInt(), body[2][2].asInt(), body[2][3].asInt());
    int diffX = newAnchor.x - bodyNodes[2].x;
    int diffY = newAnchor.y - bodyNodes[2].y;
    int diffZ = newAnchor.z - bodyNodes[2].z;

    bodyNodes[0].x = body[0][0].asInt();
    bodyNodes[0].y = body[0][1].asInt();
    bodyNodes[0].z = body[0][2].asInt();
    bodyNodes[0].score = body[0][3].asInt();

    bodyNodes[1].x = body[1][0].asInt();
    bodyNodes[1].y = body[1][1].asInt();
    bodyNodes[1].z = body[1][2].asInt();
    bodyNodes[1].score = body[1][3].asInt();

    bodyNodes[2] = newAnchor;

    for (int i = 4; i < 13; ++i) {
        bodyNodes[i].x = body[i][0].asInt();
        bodyNodes[i].y = body[i][1].asInt();
        bodyNodes[i].z = body[i][2].asInt();
        bodyNodes[i].score = body[i][3].asInt();
    }

    for (int i = 13; i < 19; ++i) {
        bodyNodes[i].x += diffX;
        bodyNodes[i].y += diffY;
        bodyNodes[i].z += diffZ;
    }
}

void Skeleton::updateMinor(const Json::Value &bodyMsg)
{
    if (bodyMsg["body_nodes"].empty()) {
        minorExist = false;
        return;
    }

    minorExist = true;
    const Json::Value &body = bodyMsg["body_nodes"];
    Node newAnchor = Node(body[2][0].asInt(), body[2][1].asInt(), body[2][2].asInt(), body[2][3].asInt());
    int diffX = bodyNodes[2].x - newAnchor.x;
    int diffY = bodyNodes[2].y - newAnchor.y;
    int diffZ = bodyNodes[2].z - newAnchor.z;

    for (int i = 13; i < 19; ++i) {
        bodyNodes[i].x = body[i][0].asInt() + diffX;
        bodyNodes[i].y = body[i][1].asInt() + diffY;
        bodyNodes[i].z = body[i][2].asInt() + diffZ;
        bodyNodes[i].score = body[i][3].asInt();
    }
}

void Skeleton::updateHands(const Json::Value &hands)
{
    if (hands["left_hand_nodes"].empty()) {
        leftHandExist = false;
    } else {
        int score = hands["left_hand_score"].asInt();
        const Json::Value &nodes = hands["left_hand_nodes"];
        for (int i = 0; i < 21; ++i) {
            leftHandNodes[i].x = nodes[i][0].asInt();
            leftHandNodes[i].y = nodes[i][1].asInt();
            leftHandNodes[i].z = nodes[i][2].asInt();
            leftHandNodes[i].score = score;
        }
    }

    if (hands["right_hand_nodes"].empty()) {
        rightHandExist = false;
    } else {
        int score = hands["right_hand_score"].asInt();
        const Json::Value &nodes = hands["right_hand_nodes"];
        for (int i = 0; i < 21; ++i) {
            rightHandNodes[i].x = nodes[i][0].asInt();
            rightHandNodes[i].y = nodes[i][1].asInt();
            rightHandNodes[i].z = nodes[i][2].asInt();
            rightHandNodes[i].score = score;
        }
    }
}

Combine::Combine(const Json::Value &config)
{
    const char *guiIp = config["gui_ip"].asCString();
    const char *simIp = config["sim_ip"].asCString();
    int guiPort = config["gui_port"].asInt();
    int simPort = config["sim_port"].asInt();

    mainCameraId = config["main_camera_id"].asInt();
    minorCameraId = config["minor_camera_id"].asInt();
    handCameraId = config["hand_camera_id"].asInt();

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
        printf("Received main camera msg\n");
        mainCameraRecv(payload);
    } else if (cameraId == minorCameraId) {
        printf("Received minor camera msg\n");
        minorCameraRecv(payload);
    } else if (cameraId == handCameraId) {
        printf("Received hand camera msg\n");
        handCameraRecv(payload);
    } else {
        printf("Unknown Camera ID: %d\n", cameraId);
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
        skeleton.updateMain(payload);
        mainFrameId = frameId;
    }
}

void Combine::minorCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    if (frameId > minorFrameId) {
        skeleton.updateMinor(payload);
        minorFrameId = frameId;
    }
}

void Combine::handCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    if (frameId > handFrameId) {
        skeleton.updateHands(payload);
        handFrameId = frameId;
    }
}