#include "TcpConnector.hpp"

int main()
{

    TCPConnector connector;
    std::unique_ptr<TCPStream> stream = connector.connect("192.168.0.1", 8080);
    return 0;
}