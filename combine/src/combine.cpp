#include "combine.h"

#include <chrono>
#include <memory>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

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

    bodyNodes[3].x = body[3][0].asInt() + diffX;
    bodyNodes[3].y = body[3][1].asInt() + diffY;
    bodyNodes[3].z = body[3][2].asInt() + diffZ;
    bodyNodes[3].score = body[3][3].asInt();

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
        leftHandExist = true;
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
        rightHandExist = true;
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

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
        printf("Unknown Camera ID: %d\n", cameraId);
    }
}

void Combine::mainCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received main\tcamera msg\tframe ID: %ld\n", frameId);
    // printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
    if (frameId > mainFrameId) {
        skeleton.updateMain(payload);
        mainFrameId = frameId;
    }
    send();
}

void Combine::minorCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received minor\tcamera msg\tframe ID: %ld\n", frameId);
    // printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
    if (frameId > minorFrameId) {
        skeleton.updateMinor(payload);
        minorFrameId = frameId;
    }
    send();
}

void Combine::handCameraRecv(const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received hand\tcamera msg\tframe ID: %ld\n", frameId);
    // printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
    if (frameId > handFrameId) {
        skeleton.updateHands(payload);
        handFrameId = frameId;
    }
    send();
}

void Combine::sendToGuiClient(const char *buf, int len)
{
    sendto(sockfd, buf, len, 0, (struct sockaddr*)&guiAddr, addrLen);
}

void Combine::sendToSimClient(const char *buf, int len)
{
    sendto(sockfd, buf, len, 0, (struct sockaddr*)&simAddr, addrLen);
}

void Combine::send()
{
    if (mainFrameId != minorFrameId || minorFrameId != handFrameId) {
        // printf("%ld %ld %ld\n", mainCameraId, minorCameraId, handCameraId);
        return;
    }

    Json::Value bodyNodes;
    for (int i = 0; i < 19; ++i) {
        Json::Value node;
        node["x"] = skeleton.bodyNodes[i].x;
        node["y"] = skeleton.bodyNodes[i].y;
        node["z"] = skeleton.bodyNodes[i].z;
        node["score"] = skeleton.bodyNodes[i].score;
        bodyNodes.append(node);
    }

    if (!skeleton.mainExist) {
        bodyNodes[0]["score"] = 0;
        bodyNodes[1]["score"] = 0;
        bodyNodes[2]["score"] = 0;
        for (int i = 4; i < 13; ++i) {
            bodyNodes[i]["score"] = 0;
        }
    }

    if (!skeleton.minorExist) {
        bodyNodes[3]["score"] = 0;
        for (int i = 13; i < 19; ++i) {
            bodyNodes[i]["score"] = 0;
        }
    }

    Json::Value payload;
    payload["body_nodes"] = bodyNodes;

    if (skeleton.leftHandExist) {
        int leftWristX = skeleton.bodyNodes[7].x;
        int leftWristY = skeleton.bodyNodes[7].y;
        int leftWristZ = skeleton.bodyNodes[7].z;

        Json::Value leftHandNodes;
        for (int i = 0; i < 21; ++i) {
            Json::Value node;
            node["x"] = skeleton.leftHandNodes[i].x + leftWristX;
            node["y"] = skeleton.leftHandNodes[i].y + leftWristY;
            node["z"] = skeleton.leftHandNodes[i].z + leftWristZ;
            node["score"] = skeleton.leftHandNodes[i].score;
            leftHandNodes.append(node);
        }
        payload["left_hand_nodes"] = leftHandNodes;
    }
    
    if (skeleton.rightHandExist) {
        int rightWristX = skeleton.bodyNodes[11].x;
        int rightWristY = skeleton.bodyNodes[11].y;
        int rightWristZ = skeleton.bodyNodes[11].z;

        Json::Value rightHandNodes;
        for (int i = 0; i < 21; ++i) {
            Json::Value node;
            node["x"] = skeleton.rightHandNodes[i].x + rightWristX;
            node["y"] = skeleton.rightHandNodes[i].y + rightWristY;
            node["z"] = skeleton.rightHandNodes[i].z + rightWristZ;
            node["score"] = skeleton.rightHandNodes[i].score;
            rightHandNodes.append(node);
        }
        payload["right_hand_nodes"] = rightHandNodes;
    }
    printf("Sending...\n\n\n");

	payload["frame_id"] = Json::Value::Int64(mainFrameId);
    // printf("\n\n\n%s", payload.toStyledString().c_str());
	std::string jsonStr = writer.write(payload);
	sendToGuiClient(jsonStr.c_str(), jsonStr.length());
}