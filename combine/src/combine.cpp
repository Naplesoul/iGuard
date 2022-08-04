#include "combine.h"

#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip> 
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

std::chrono::system_clock::time_point stringToDateTime(const std::string &s)
{
    std::tm timeDate = {};
    std::istringstream ss(s);
    ss >> std::get_time(&timeDate, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(mktime(&timeDate));
}

static const std::chrono::system_clock::time_point firstFrameTime = stringToDateTime("2022-07-15 00::00::00");

bool BodyMetrics::update(const Json::Value &bodyMsg)
{
    if (bodyMsg["body_metrics"].empty()) {
        return false;
    }

    const Json::Value &metrics = bodyMsg["body_metrics"];
    int _armWidth = metrics["arm_width"].asInt();
	int _legWidth = metrics["leg_width"].asInt();
	int _headWidth = metrics["head_width"].asInt();
	int _torsoWidth = metrics["torso_width"].asInt();

    if (_armWidth > 50 && _armWidth < 200 &&
        _legWidth > 80 && _legWidth < 300 &&
        _headWidth > 120 && _headWidth < 300 &&
        _torsoWidth > 250 && _torsoWidth < 600) {
        
        armWidth = _armWidth;
        legWidth = _legWidth;
        headWidth = _headWidth;
        torsoWidth = _torsoWidth;

        json = metrics;
        return true;
    }

    return false;
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

Combine::Combine(const Json::Value &config): frameTime(0)
{
    const char *guiIp = config["gui_ip"].asCString();
    const char *simIp = config["sim_ip"].asCString();
    int guiPort = config["gui_port"].asInt();
    int simPort = config["sim_port"].asInt();

    mainCameraId = config["main_camera_id"].asInt();
    minorCameraId = config["minor_camera_id"].asInt();
    handCameraId = config["hand_camera_id"].asInt();

    frameTime = std::chrono::nanoseconds(1000000000 / config["fps"].asInt());

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

void Combine::recv(const struct sockaddr_in &clientAddr, const Json::Value &payload)
{
    int cameraId = payload["camera_id"].asInt();
    if (cameraId == mainCameraId) {
        mainCameraRecv(clientAddr, payload);
    } else if (cameraId == minorCameraId) {
        minorCameraRecv(clientAddr, payload);
    } else if (cameraId == handCameraId) {
        handCameraRecv(clientAddr, payload);
    } else {
        printf("Unknown Camera ID: %d\n", cameraId);
    }
}

void Combine::mainCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received main\tcamera msg\tframe ID: %ld", frameId);
    if (frameId <= mainFrameId) {
        return;
    }

    int64_t diff = timeDiff(frameId);
    if (diff < 64 && diff > -64) {
        avgMainDiff = 0.3 * avgMainDiff + 0.7 * diff;
    }

    skeleton.updateMain(payload);
    if (enableBodyScan && frameId > firstFrameId + 50) {
        enableBodyScan = !bodyMetrics.update(payload);
    }
    mainFrameId = frameId;
    if (frameId % 16 == 0) {
        std::string feedback = "{\"offset\":" + std::to_string(avgMainDiff);
        if (enableBodyScan) {
            feedback += ",\"body_scan\":true}";
        } else {
            feedback += ",\"body_scan\":false}";
        }
        sendto(sockfd, feedback.data(), feedback.length(), 0, (struct sockaddr*)&clientAddr, addrLen);
        avgMainDiff = 0;
    }
    send();
}

void Combine::minorCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received minor\tcamera msg\tframe ID: %ld", frameId);
    if (frameId <= minorFrameId) {
        return;
    }

    int64_t diff = timeDiff(frameId);
    if (diff < 64 && diff > -64) {
        avgMinorDiff = 0.3 * avgMinorDiff + 0.7 * diff;
    }
    
    skeleton.updateMinor(payload);
    if (enableBodyScan && frameId > firstFrameId + 50) {
        enableBodyScan = !bodyMetrics.update(payload);
    }
    minorFrameId = frameId;
    if (frameId % 16 == 0) {
        std::string feedback = "{\"offset\":" + std::to_string(avgMinorDiff);
        if (enableBodyScan) {
            feedback += ",\"body_scan\":true}";
        } else {
            feedback += ",\"body_scan\":false}";
        }
        sendto(sockfd, feedback.data(), feedback.length(), 0, (struct sockaddr*)&clientAddr, addrLen);
        avgMinorDiff = 0;
    }
    send();
}

void Combine::handCameraRecv(const struct sockaddr_in &clientAddr, const Json::Value &payload)
{
    int64_t frameId = payload["frame_id"].asInt64();
    printf("Received hand\tcamera msg\tframe ID: %ld", frameId);
    if (frameId <= handFrameId) {
        return;
    }
    
    int64_t diff = timeDiff(frameId);
    if (diff < 64 && diff > -64) {
        avgHandDiff = 0.3 * avgHandDiff + 0.7 * diff;
    }

    skeleton.updateHands(payload);
    if (enableBodyScan && frameId > firstFrameId + 50) {
        enableBodyScan = !bodyMetrics.update(payload);
    }

    machineState.running = payload["running"].asBool();
    machineState.carriageX = payload["carriage_x"].asInt();
    machineState.carriageZ = payload["carriage_z"].asInt();

    handFrameId = frameId;
    if (frameId % 16 == 0) {
        std::string feedback = "{\"offset\":" + std::to_string(avgHandDiff);
        if (enableBodyScan) {
            feedback += ",\"body_scan\":true}";
        } else {
            feedback += ",\"body_scan\":false}";
        }
        sendto(sockfd, feedback.data(), feedback.length(), 0, (struct sockaddr*)&clientAddr, addrLen);
        avgHandDiff = 0;
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

    payload["running"] = machineState.running;
    payload["carriage_x"] = machineState.carriageX;
    payload["carriage_z"] = machineState.carriageZ;
    if (!enableBodyScan) {
        payload["body_metrics"] = bodyMetrics.json;
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
    if (firstFrameId < 0) {
        firstFrameId = mainFrameId;
    }
	sendToGuiClient();
    sendToSimClient();
    printf("\n\n");
}

int64_t Combine::timeDiff(int64_t frameId)
{
    auto now = std::chrono::system_clock::now();
    printf("\t\tarrival time: %ldms\n", (now - firstFrameTime).count() / 1000000);
    auto diff = firstFrameTime + frameId * frameTime - now;
    return diff.count() / 1000000;
}