#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>

using namespace std;

class TCPStream
{
    class TCPStreamKey
    {
        friend class TCPConnector;

    private:
        TCPStreamKey() {};
        TCPStreamKey(TCPStreamKey const &) = default;
    };

public:
    explicit TCPStream(int sd, struct sockaddr_in *address, TCPStreamKey)
    {
        char ip[50];
        inet_ntop(PF_INET, (struct in_addr *)&(address->sin_addr.s_addr),
                  ip, sizeof(ip) - 1);
        m_peerIp = ip;
        m_peerPort = ntohs(address->sin_port);
    }

private:
    int m_sd;
    string m_peerIp;
    int m_peerPort;
};