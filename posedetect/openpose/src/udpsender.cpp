#include "udpsender.h"

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

void UDPSender::sendPoseToServer(const Pose3D &pose3d)
{
	std::string jsonStr = pose3d.toJsonString();
	sendToServer(jsonStr.c_str(), jsonStr.length());
}