#include "client.h"

#include <thread>
#include <pthread.h>
#include <iostream>
#include <eigen3/Eigen/Core>
#include <jsoncpp/json/json.h>

#define BUFFER_SIZE 1024

UDPClient::UDPClient(const std::string &addr, uint16_t port)
{
	addrLen = sizeof(struct sockaddr_in);
	memset(&serverAddr, 0, addrLen);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(addr.c_str());

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Fail to setup socket\n");
	}
}

UDPClient::~UDPClient()
{

}

void UDPClient::sendToServer(const void *buf, int len)
{
    int i = sendto(sockfd, buf, len, 0, (struct sockaddr*)&serverAddr, addrLen);
	printf("Sending %d bytes...\n", i);
}

SkeletonClient::SkeletonClient(const std::string &addr, uint16_t port, int _cameraId,
						   void _updateOffset(int64_t), Eigen::Matrix4f _M_inv)
	: UDPClient(addr, port), cameraId(_cameraId), M_inv(_M_inv), updateOffset(_updateOffset)
{
	feedbackThread = std::thread(&SkeletonClient::pollFeedback, this);
}

SkeletonClient::~SkeletonClient()
{
    pthread_cancel(feedbackThread.native_handle());
	feedbackThread.join();
}

void SkeletonClient::pollFeedback()
{
	char buf[BUFFER_SIZE];

    while (true) {
        memset(buf, 0, BUFFER_SIZE);
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int count = recvfrom(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &len);
        if (count < 0) {
            printf("Fail to receive from server\n");
            continue;
        }
        Json::Value payload;
        reader.parse(buf, payload);
		int64_t offset = payload["offset"].asInt64();
		updateOffset(offset);
    }
}

void SkeletonClient::sendEmpty(uint64_t frameId)
{
	Json::Value payload;
	payload["camera_id"] = cameraId;
	payload["frame_id"] = Json::Value::Int64(frameId);
	
	printf("\n\n\n%s", payload.toStyledString().c_str());
	std::string jsonStr = writer.write(payload);
	sendToServer(jsonStr.c_str(), jsonStr.length());
}

void SkeletonClient::sendSkeleton(uint64_t frameId, const tdv::nuitrack::Skeleton &newSkeleton)
{
	Json::Value bodyNodes;
	for (int i = 0; i < 25; ++i) {
		switch (i)
		{
		case 0:
		case 10:
		case 11:
		case 16:
		case 20:
		case 24:
			continue;
		default:
			Json::Value node;
			Eigen::Vector4f cam4(newSkeleton.joints[i].real.x, newSkeleton.joints[i].real.y, newSkeleton.joints[i].real.z, 1);
			Eigen::Vector4f real4 = M_inv * cam4;
			int score = newSkeleton.joints[i].confidence * 100;

			node.append(int(real4(0)));
			node.append(int(real4(1)));
			node.append(int(real4(2)));
			node.append(score);

			// node["x"] = int(real4(0));
			// node["y"] = int(real4(1));
			// node["z"] = int(real4(2));
			// node["score"] = newSkeleton.joints[i].confidence;

			bodyNodes.append(node);
		}
	}

	Json::Value payload;
	payload["camera_id"] = cameraId;
	payload["frame_id"] = Json::Value::Int64(frameId);
	payload["body_nodes"] = bodyNodes;

	printf("\n\n\n%s", payload.toStyledString().c_str());
	std::string jsonStr = writer.write(payload);
	sendToServer(jsonStr.c_str(), jsonStr.length());
}