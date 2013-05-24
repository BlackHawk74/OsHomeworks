#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

int main(int, char **)
{
    addrinfo hints;
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    {
        int ainfo = getaddrinfo(NULL, "8822", &hints, &result);
        if (ainfo != 0)
        {
            std::cerr << "Error in getaddrinfo()" << std::endl;
            _exit(EXIT_FAILURE);
        }
    }

    int sockfd = socket(result->ai_family, result->ai_socktype,
                           result->ai_protocol);

    if (sockfd == -1)
    {
        std::cerr << "Error opening socket\n";
        _exit(EXIT_FAILURE);
    }

    {
        int optval = 1;
        int ss = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (ss != 0)
        {
            std::cerr << "Socket options not set\n";
        }
    }



}

