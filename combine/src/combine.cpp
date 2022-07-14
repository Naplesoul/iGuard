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
    int diffX, diffY, diffZ;
    const Json::Value &body = bodyMsg["body_nodes"];

    diffX = body[13][0].asInt() - bodyNodes[13].x;
    diffY = body[13][1].asInt() - bodyNodes[13].y;
    diffZ = body[13][2].asInt() - bodyNodes[13].z;

    bodyNodes[14].x += diffX;
    bodyNodes[14].y += diffY;
    bodyNodes[14].z += diffZ;

    bodyNodes[15].x += diffX;
    bodyNodes[15].y += diffY;
    bodyNodes[15].z += diffZ;
    
    diffX = body[16][0].asInt() - bodyNodes[16].x;
    diffY = body[16][1].asInt() - bodyNodes[16].y;
    diffZ = body[16][2].asInt() - bodyNodes[16].z;

    bodyNodes[17].x += diffX;
    bodyNodes[17].y += diffY;
    bodyNodes[17].z += diffZ;

    bodyNodes[18].x += diffX;
    bodyNodes[18].y += diffY;
    bodyNodes[18].z += diffZ;

    for (int i = 0; i < 14; ++i) {
        bodyNodes[i].x = body[i][0].asInt();
        bodyNodes[i].y = body[i][1].asInt();
        bodyNodes[i].z = body[i][2].asInt();
        bodyNodes[i].score = body[i][3].asInt();
    }

    bodyNodes[16].x = body[16][0].asInt();
    bodyNodes[16].y = body[16][1].asInt();
    bodyNodes[16].z = body[16][2].asInt();
    bodyNodes[16].score = body[16][3].asInt();
}

void Skeleton::updateMinor(const Json::Value &bodyMsg)
{
    if (bodyMsg["body_nodes"].empty()) {
        minorExist = false;
        return;
    }

    minorExist = true;
    int diffX, diffY, diffZ;
    const Json::Value &body = bodyMsg["body_nodes"];

    diffX = bodyNodes[13].x - body[16][0].asInt();
    diffY = bodyNodes[13].y - body[16][1].asInt();
    diffZ = bodyNodes[13].z - body[16][2].asInt();

    bodyNodes[14].x = body[17][0].asInt() + diffX;
    bodyNodes[14].y = body[17][1].asInt() + diffY;
    bodyNodes[14].z = body[17][2].asInt() + diffZ;
    bodyNodes[14].score = body[17][3].asInt();

    bodyNodes[15].x = body[18][0].asInt() + diffX;
    bodyNodes[15].y = body[18][1].asInt() + diffY;
    bodyNodes[15].z = body[18][2].asInt() + diffZ;
    bodyNodes[15].score = body[18][3].asInt();

    diffX = bodyNodes[16].x - body[13][0].asInt();
    diffY = bodyNodes[16].y - body[13][1].asInt();
    diffZ = bodyNodes[16].z - body[13][2].asInt();

    bodyNodes[17].x = body[14][0].asInt() + diffX;
    bodyNodes[17].y = body[14][1].asInt() + diffY;
    bodyNodes[17].z = body[14][2].asInt() + diffZ;
    bodyNodes[17].score = body[14][3].asInt();

    bodyNodes[18].x = body[15][0].asInt() + diffX;
    bodyNodes[18].y = body[15][1].asInt() + diffY;
    bodyNodes[18].z = body[15][2].asInt() + diffZ;
    bodyNodes[18].score = body[15][3].asInt();
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
    printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
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
    printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
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
    printf("%ldms\n", std::chrono::system_clock::now().time_since_epoch().count() / 1000000);
    if (frameId > handFrameId) {
        skeleton.updateHands(payload);
        handFrameId = frameId;
    }
    send();
}

void Combine::sendToGuiClient()
{
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

	payload["frame_id"] = Json::Value::Int64(mainFrameId);
    printf("Sending to GUI Client...\n");
    // printf("%s\n\n\n", payload.toStyledString().c_str());
	std::string jsonStr = writer.write(payload);
    sendto(sockfd, jsonStr.data(), jsonStr.length(), 0, (struct sockaddr*)&guiAddr, addrLen);
}

void Combine::sendToSimClient()
{
    if (!skeleton.mainExist || !skeleton.minorExist) {
        return;
    }
    Json::Value bodyNodes;
    for (int i = 0; i < 19; ++i) {
        Json::Value node;
        node.append(skeleton.bodyNodes[i].x);
        node.append(skeleton.bodyNodes[i].y);
        node.append(skeleton.bodyNodes[i].z);
        bodyNodes.append(node);
    }
    Json::Value payload;
    payload["body_nodes"] = bodyNodes;
    payload["frame_id"] = Json::Value::Int64(mainFrameId);
    printf("Sending to SIM Client...\n");
    // printf("%s\n\n\n", payload.toStyledString().c_str());
    std::string jsonStr = writer.write(payload);
    sendto(sockfd, jsonStr.data(), jsonStr.length(), 0, (struct sockaddr*)&simAddr, addrLen);
}

void Combine::send()
{
    if (mainFrameId != minorFrameId || minorFrameId != handFrameId) {
        // printf("%ld %ld %ld\n", mainCameraId, minorCameraId, handCameraId);
        return;
    }
	sendToGuiClient();
    sendToSimClient();
    printf("\n\n");
}