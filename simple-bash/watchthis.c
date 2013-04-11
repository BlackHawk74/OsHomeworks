#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        puts("Usage: watchthis INTERVAL COMMAND");
        return 1;
    }

    int interval = atoi(argv[1]);
    if (interval <= 0)
    {
        puts("Interval must be greater than zero");
        return 2;
    }
    argv[1] = "-c";

    int pipefd[2];
    pipe(pipefd);

    char buf[100];
    int read_result;
    while (1) {
        if (fork())
        {
            while(read_result = read(pipefd[1], buf, sizeof(buf)))
            {
                int written;
                for (written = 0; written < read_result; )
                {
                    written += write(STDOUT_FILENO, buf + written, sizeof(buf) - written);
                } 
            }
            sleep(interval);

        } else {
            dup2(pipefd[0], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            execv("/bin/sh", &argv[1]);
        }
    }
    return 0;
}

