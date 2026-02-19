#pragma once
#include <netinet/in.h>
#include <memory>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "TcpStream.hpp"

class TCPConnector
{
public:
    std::unique_ptr<TCPStream> connect(const char *server, int port)
    {
        struct sockaddr_in address;

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        inet_pton(PF_INET, server, &(address.sin_addr));

        int sd = socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(sd, (struct sockaddr *)&address, sizeof(address)) != 0)
        {
            return nullptr;
        }
        return std::make_unique<TCPStream>(TCPStream{sd, &address, {}});
    };
};