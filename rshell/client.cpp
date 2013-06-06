#include <iostream>
#include <poll.h>
#include <errno.h>
#include <pty.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <cstdio>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include "protocol.h"

const int BACKLOG = 64;

int main(int argc, char ** argv) 
{
    if (argc < 2)
    {
        std::cerr << "Host not specified" << std::endl;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));

    // Setting connection options
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = 0;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */

    // Getting address info
    addrinfo *result;
    {
        int ainfo = getaddrinfo(argv[1], "8822", &hints, &result);
        if (ainfo != 0)
        {
            std::cerr << "Could not resolve host\n";
            _exit(EXIT_FAILURE);
        }
    }

    // Creating socket
    int sockfd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);

    if (sockfd == -1)
    {
        std::cerr << "Could not create socket\n";
        _exit(EXIT_FAILURE);
    }

    // Setting socket option to reuse address
    {
        int optval = 1;
        int ss = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        if (ss != 0)
        {
            std::cerr << "Cannot set socket options\n";
            _exit(EXIT_FAILURE);
        }
    }

    if (connect(sockfd, result->ai_addr, rp->ai_addrlen) == -1)
    {
        std::cerr << "Could not connect\n";
        _exit(EXIT_FAILURE);
    }

    pollfd socket_poll[2];
    socket_poll[0].fd = sockfd;
    socket_poll[1].fd = STDIN_FILENO;

    const short ERR_CODES = POLLERR | POLLHUP | POLLNVAL | POLLRDHUP;
    socket_poll[0].events = socket_poll[1].events = POLLIN | ERR_CODES;

    while (true)
    {
        int res = poll(socket_poll, 2, -1);
        if ((socket_poll[0].revents & ERR_CODES) || (socket_poll[1].revents & ERR_CODES))
        {
            std::cerr << "Connection error\n";
            _exit(EXIT_FAILURE);
        }

        if (socket_poll[0].revents & POLLIN)
        {
            int
        }
    }
}
