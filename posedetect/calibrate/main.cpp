#include <opencv2/opencv.hpp>
#include <jsoncpp/json/json.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>

#include <set>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

cv::Scalar redMin(0, 160, 100);
cv::Scalar redMax(250, 250, 250);

bool running = true;

struct Edge
{
    int s, d;
    float length;
};

void signalHandler(int signal)
{
    running = false;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, &signalHandler);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(50008);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int addrLen = sizeof(struct sockaddr_in);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    Json::FastWriter writer;

    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;

    if (freenect2.enumerateDevices() == 0) {
        std::cout << "no device connected!" << std::endl;
        return -1;
    }
    std::string serial = freenect2.getDefaultDeviceSerialNumber();
    std::cout << serial << std::endl;

    pipeline = new libfreenect2::CpuPacketPipeline();
    dev = freenect2.openDevice(serial, pipeline);

    int types = 0;
    types |= libfreenect2::Frame::Color | libfreenect2::Frame::Depth;
    libfreenect2::SyncMultiFrameListener listener(types);
    libfreenect2::FrameMap frames;
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);

    if (!dev->start())
        return -1;

    libfreenect2::Registration *registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
    libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4);

    while (running) {
        if (!listener.waitForNewFrame(frames, 10 * 1000)) {
            std::cout << "timeout!" << std::endl;
            return -1;
        }
        libfreenect2::Frame *colorFrame = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *depthFrame = frames[libfreenect2::Frame::Depth];
        registration->apply(colorFrame, depthFrame, &undistorted, &registered);
        cv::Mat depth(undistorted.height, undistorted.width, CV_32FC1, undistorted.data);
        cv::Mat color(registered.height, registered.width, CV_8UC4, registered.data);
        cv::imwrite("color.png", color);
        cv::cvtColor(color, color, cv::COLOR_BGRA2BGR);
        cv::cvtColor(color, color, cv::COLOR_BGR2Lab);
        cv::inRange(color, redMin, redMax, color);
        std::vector< std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(color, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::RotatedRect> anchors;
        for (auto &contour : contours) {
            cv::RotatedRect rotatedRect = cv::minAreaRect(contour);
            float area = rotatedRect.size.area();
            anchors.push_back(rotatedRect);
        }
        if (anchors.size() < 4) {
            listener.release(frames);
            continue;
        }
        std::sort(anchors.begin(), anchors.end(), [&](const cv::RotatedRect &a, const cv::RotatedRect &b) -> bool {
            return a.size.area() > b.size.area();
        });
        anchors.resize(4);
        std::vector<Edge> edges;
        std::vector<std::vector<Edge>> graph;
        std::vector<float> sum(4, 0);
        graph.resize(4);
        for (int i = 0; i < 4; ++i) {
            for (int j = i + 1; j < 4; ++j) {
                float xi, yi, zi, xj, yj, zj;
                registration->getPointXYZ(&undistorted, anchors[i].center.y, anchors[i].center.x, xi, yi, zi);
                registration->getPointXYZ(&undistorted, anchors[j].center.y, anchors[j].center.x, xj, yj, zj);
                float dist = sqrt(pow((xi - xj), 2) + pow((yi - yj), 2) + pow((zi - zj), 2));
                // printf("(%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), %.2f\n", xi, yi, zi, xj, yj, zj, dist);
                Edge edge{.s = i, .d = j, .length = dist};
                edges.push_back(edge);
                graph[i].push_back(edge);
                graph[j].push_back(edge);
                sum[i] += dist;
                sum[j] += dist;
            }
        }
        std::sort(edges.begin(), edges.end(), [&](const Edge &a, const Edge &b) -> bool {
            return a.length > b.length;
        });

        int idxA, idxB, idxC, idxD;
        std::set<int> remain = {0,1,2,3};
        if (sum[edges[0].s] > sum[edges[0].d]) {
            idxD = edges[0].s;
            idxB = edges[0].d;
        } else {
            idxD = edges[0].d;
            idxB = edges[0].s;
        }
        remain.erase(idxB);
        remain.erase(idxD);
        if (sum[*remain.begin()] > sum[*remain.rbegin()]) {
            idxA = *remain.begin();
            idxC = *remain.rbegin();
        } else {
            idxA = *remain.rbegin();
            idxC = *remain.begin();
        }

        printf("idx: %d, %d, %d, %d\n", idxA, idxB, idxC, idxD);
        float xa, ya, za, xb, yb, zb, xc, yc, zc, xd, yd, zd;
        registration->getPointXYZ(&undistorted, anchors[idxA].center.y, anchors[idxA].center.x, xa, ya, za);
        registration->getPointXYZ(&undistorted, anchors[idxB].center.y, anchors[idxB].center.x, xb, yb, zb);
        registration->getPointXYZ(&undistorted, anchors[idxC].center.y, anchors[idxC].center.x, xc, yc, zc);
        registration->getPointXYZ(&undistorted, anchors[idxD].center.y, anchors[idxD].center.x, xd, yd, zd);

        Json::Value cam;
        Json::Value x, y, z, one;
        x.append(-xa * 1000);
        x.append(-xb * 1000);
        x.append(-xc * 1000);
        x.append(-xd * 1000);

        y.append(-ya * 1000);
        y.append(-yb * 1000);
        y.append(-yc * 1000);
        y.append(-yd * 1000);

        z.append(za * 1000);
        z.append(zb * 1000);
        z.append(zc * 1000);
        z.append(zd * 1000);

        one.append(1);
        one.append(1);
        one.append(1);
        one.append(1);

        cam.append(x);
        cam.append(y);
        cam.append(z);
        cam.append(one);

        std::string str = writer.write(cam);
        int i = sendto(sockfd, str.data(), str.length(), 0, (struct sockaddr*)&serverAddr, addrLen);
        std::cout << "cam:\n" << cam << "\n\n";
        listener.release(frames);
    }

    dev->stop();
    dev->close();
}
