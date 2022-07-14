#include "client.h"

#include <iostream>
#include <eigen3/Eigen/Core>
#include "json.h"

DetectClient::DetectClient(const char* addr, uint16_t port, int _cameraId,
						   Eigen::Matrix4f _M_inv, float _smooth):
	frameId(0), cameraId(_cameraId), smooth(_smooth), smoothRest(1 - _smooth), M_inv(_M_inv)
{

	//addr_len = sizeof(SOCKADDR_IN);
	//memset(&serveraddr, 0, addr_len);

	//send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//if (send_socket == SOCKET_ERROR) {
	//	std::cout << "socket Error = " << WSAGetLastError() << std::endl;
	//	return;
	//}

	//serveraddr.sin_family = AF_INET;
	//serveraddr.sin_port = htons(port);
	//serveraddr.sin_addr.s_addr = inet_addr(addr);
}

DetectClient::~DetectClient()
{
	//关闭socket连接
	//closesocket(send_socket);
	//清理
	//WSACleanup();
}

void DetectClient::sendToServer(const char* buf, int len){
	//sendto(send_socket, buf, len, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR));
}

void DetectClient::sendPoseToServer()
{
	Json::Value nodes;
	for (int i = 0; i < 25; ++i) {
		Json::Value node;

		node["x"] = int(skeleton.joints[i].x);
		node["y"] = int(skeleton.joints[i].y);
		node["z"] = int(skeleton.joints[i].z);
        node["score"] = skeleton.joints[i].score;

		nodes.append(node);
	}

	Json::Value json;
	json["camera_id"] = cameraId;
	json["frame_id"] = Json::Value::Int64(frameId);
	json["node_num"] = 25;
	json["nodes"] = nodes;

	std::cout << "\n\n\nSending\n" << json.toStyledString();
	std::string jsonStr = writer.write(json);
	sendToServer(jsonStr.c_str(), jsonStr.length());
}

void DetectClient::update(uint64_t _frameId, const tdv::nuitrack::Skeleton &newSkeleton)
{
	frameId = _frameId;
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