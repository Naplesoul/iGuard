#include <string.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jsoncpp/json/json.h>

#include "combine.h"

#define BUFFER_SIZE 4096

Json::Reader reader;
Combine *combine = nullptr;

void listen(int fd)
{
    char buf[BUFFER_SIZE];

    while (true) {
        memset(buf, 0, BUFFER_SIZE);
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);

        int count = recvfrom(fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &len);
        if (count < 0) {
            printf("Fail to receive from client\n");
            continue;
        }
        Json::Value payload;
        reader.parse(buf, payload);
        combine->recv(payload);
    }
}

int main(int argc, char **argv)
{
    printf("Usage: combine [path/to/config.json]\n");

    if (argc < 2) {
        printf("Missing config json\n");
        exit(-1);
    }

    Json::Value config;
    std::fstream file(argv[1]);
    Json::Reader reader;
    reader.parse(file, config);

    int listenPort = config["listen_port"].asInt();
    combine = new Combine(config);

    int listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in listenAddr;
    memset(&listenAddr, 0, sizeof(listenAddr));
    
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(listenPort);

    int b = bind(listenfd, (struct sockaddr *)&listenAddr, sizeof(listenAddr));
    if (b < 0) {
        printf("Fail to bind port\n");
        exit(-1);
    }
    printf("Combine sever listening at port %d...\n", listenPort);

    listen(listenfd);
}