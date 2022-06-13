#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

class udpsender
{
private:
    int sockfd;
    int addr_len;
    struct sockaddr_in serveraddr;
public:
    void send_to_server(const char* buf, int len);

    udpsender(const char* addr, uint16_t port);
    ~udpsender();
};

