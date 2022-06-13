#include "udpsender.h"
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

void UDPSender::send_to_server(const char* buf, int len){
    sendto(sockfd, buf, len, 0, (struct sockaddr*)&serveraddr, addr_len);
}

void UDPSender::sendPoseToServer(Pose3D &pose3d)
{
	Json::Value v;
	v["node_num"] = BODY_PART_CNT;
	
	Json::Value nodes;
	for (int i = 0; i < BODY_PART_CNT; ++i) {
		Json::Value node;
		node["x"] = pose3d[BodyPart(i)].x;
		node["y"] = pose3d[BodyPart(i)].y;
		node["z"] = pose3d[BodyPart(i)].z;

		nodes.append(node);
	}
	v["nodes"] = nodes;

	std::string s = v.toStyledString();
	send_to_server(s.c_str(), s.length());
}
