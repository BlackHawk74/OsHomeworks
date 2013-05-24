#include <iostream>
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

const int BACKLOG = 64;

void write_all(int fd, const char * buf, int count)
{
    int written;
    
    // write() has no guarantee to write everything
    for (written = 0; written < count;)
    {
        int t = write(fd, buf + written, count - written);
        if (t == -1) // Write failed
        {
            _exit(4);
        }
        written += t;
    }
}

pid_t pid;

void handler(int) {
    kill(pid, SIGINT);
}


int main(int, char **)
{
    pid = fork();
    if (pid)
    {
        signal(SIGINT, handler);
        std::cout << "Started daemon, pid: " << pid << std::endl;
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Daemon stopped" << std::endl;
    } else 
    {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        setsid();

        addrinfo hints;
        addrinfo *result;

        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        hints.ai_protocol = 0;          /* Any protocol */

        {
            int ainfo = getaddrinfo(NULL, "8822", &hints, &result);
            if (ainfo != 0)
            {
                _exit(EXIT_FAILURE);
            }
        }

        int sockfd = socket(result->ai_family, result->ai_socktype,
                               result->ai_protocol);

        if (sockfd == -1)
        {
            _exit(EXIT_FAILURE);
        }

        {
            int optval = 1;
            int ss = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
            if (ss != 0)
            {
                _exit(EXIT_FAILURE);
            }
        }

        if (bind(sockfd, result->ai_addr, result->ai_addrlen))
        {
            _exit(EXIT_FAILURE);
        }

        if (listen(sockfd, BACKLOG))
        {
            _exit(EXIT_FAILURE);
        }

        while (true) 
        {
            int fd = accept(sockfd, result->ai_addr, &result->ai_addrlen);
            if (fork())
            {
                close(fd);
            } else
            {
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);

                write_all(fd, "Hello, world!\n", 14);       
                close(fd);
                _exit(0);
            }
        }
        
    }
}

