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

    int pipefd[2];
    size_t buf_size = 100;
    size_t used = 0;
    char * buf = (char*) malloc(buf_size);
    int read_result;
    while (1) {
        pipe(pipefd);
        if (fork())
        {
            close(pipefd[1]);
            wait(NULL);
            used = 0;
            while((read_result = read(pipefd[0], buf + used, buf_size - used)) > 0)
            {
//                write(1, "read\n", 5);
                used += read_result;
                if (used == buf_size)
                {
//                    write(1, "realloc\n", 8);
                    buf_size *= 2;
                    buf = (char*) realloc((void*) buf, buf_size);
                }
            }

            int written;
            for (written = 0; written < used; )
            {
                written += write(STDOUT_FILENO, buf + written, used - written);
            }

            close(pipefd[0]);
            sleep(interval);

        } else {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(argv[2], &argv[2]);
        }
    }
    return 0;
}

