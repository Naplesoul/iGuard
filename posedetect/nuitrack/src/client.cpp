#include "client.h"

#include <iostream>
#include <eigen3/Eigen/Core>
#include <jsoncpp/json/json.h>

DetectClient::DetectClient(const char* addr, uint16_t port,
						   Eigen::Matrix4f _M_inv, float _smooth):
	smooth(_smooth), smoothRest(1 - _smooth), M_inv(_M_inv)
{
	addr_len = sizeof(struct sockaddr_in);
	memset(&serveraddr, 0, addr_len);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("fail to setup socket");
	}
}

DetectClient::~DetectClient()
{
    
}

void DetectClient::sendToServer(const char* buf, int len){
    sendto(sockfd, buf, len, 0, (struct sockaddr*)&serveraddr, addr_len);
}

void DetectClient::sendPoseToServer()
{
	Json::Value json;
	json["node_num"] = 25;
	Json::Value nodes;
	for (int i = 0; i < 25; ++i) {
		Json::Value node;

		node["x"] = int(skeleton.joints[i].x);
		node["y"] = int(skeleton.joints[i].y);
		node["z"] = int(skeleton.joints[i].z);
        node["score"] = skeleton.joints[i].score;

		nodes.append(node);
	}
	json["nodes"] = nodes;

	std::cout << "\n\n\nSending\n" << json.toStyledString();
	std::string jsonStr = writer.write(json);
	sendToServer(jsonStr.c_str(), jsonStr.length());
}

void DetectClient::update(const tdv::nuitrack::Skeleton &newSkeleton)
{
	for (int i = 0; i < 25; ++i) {
		Eigen::Vector4f cam4(newSkeleton.joints[i].real.x, newSkeleton.joints[i].real.y, newSkeleton.joints[i].real.z, 1);
		Eigen::Vector4f real4 = M_inv * cam4;

		skeleton.joints[i].x = smoothRest * real4(0) + smooth * skeleton.joints[i].x;
		skeleton.joints[i].y = smoothRest * real4(1) + smooth * skeleton.joints[i].y;
		skeleton.joints[i].z = smoothRest * real4(2) + smooth * skeleton.joints[i].z;
		skeleton.joints[i].score = newSkeleton.joints[i].confidence;
	}

	sendPoseToServer();
}