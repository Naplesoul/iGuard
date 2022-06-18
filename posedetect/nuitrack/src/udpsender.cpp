#include "udpsender.h"

#include <iostream>
#include <jsoncpp/json/json.h>

UDPSender::UDPSender(const char* addr, uint16_t port)
{
	addr_len = sizeof(struct sockaddr_in);
	memset(&serveraddr, 0, addr_len);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("fail to setup socket");
	}
}

UDPSender::~UDPSender()
{
    
}

void UDPSender::sendToServer(const char* buf, int len){
    sendto(sockfd, buf, len, 0, (struct sockaddr*)&serveraddr, addr_len);
}

void UDPSender::sendPoseToServer(const tdv::nuitrack::Skeleton &skeleton)
{
	Json::Value json;
	json["node_num"] = 25;
	Json::Value nodes;
	for (int i = 0; i < 25; ++i) {
		Json::Value node;
		node["x"] = skeleton.joints[i].real.x / 1000;
		node["y"] = skeleton.joints[i].real.y / 1000;
		node["z"] = skeleton.joints[i].real.z / 1000;
        node["score"] = skeleton.joints[i].confidence;

		nodes.append(node);
	}
	json["nodes"] = nodes;

	std::cout << "\n\n\nSending\n" << json.toStyledString();
	std::string jsonStr = Json::FastWriter().write(json);
	sendToServer(jsonStr.c_str(), jsonStr.length());
}