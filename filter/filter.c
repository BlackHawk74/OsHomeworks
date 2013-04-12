#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void write_all(int fd, char * buf, size_t count)
{
    int written;
    for (written = 0; written < count;)
    {
        written += write(fd, buf + written, count - written);
    }
}

int main(int argc, char** argv)
{
    int buf_size = 4096;
    char delim = '\n';
    int opt_count = 1;
    
    while (1)
    {
        int next_opt = getopt(argc, argv, "+nzb:");
        if (next_opt == -1)
        {
            break;
        }

        switch (next_opt)
        {
        case 'n':
            delim = '\n';
            break;
        case 'z':
            delim = 0;
            break;
        case 'b':
            buf_size = atoi(optarg);
            if (buf_size <= 0)
            {
                return 2;
            }
            opt_count++;
            break;
        }
        opt_count++;
    }

    if (argc > opt_count && strcmp(argv[opt_count], "--") == 0) {
        opt_count++;
    }
    
    if (opt_count >= argc)
    {
        puts("No command provided");
        return 1;
    }


    buf_size++;
    char * buf = malloc(sizeof(char) * buf_size);

    size_t buf_used = 0;
    int read_count;

    char** cmd_argv = malloc(sizeof(char*) * (argc - opt_count + 2));
    int j, cmd_argc = 0;
    for (j = opt_count; j < argc; j++)
    {
        cmd_argv[cmd_argc++] = argv[j];
    }
    cmd_argv[cmd_argc + 1] = NULL;

    int eof_reached = 0;

    while (1)
    {
        read_count = read(STDIN_FILENO, buf + buf_used, buf_size - buf_used);
        if (!read_count) {
            eof_reached = 1;
            if (buf_used == 0)
            {
                break;
            }
            buf[buf_used] = delim;
            read_count++;
        }
        size_t i;
        size_t buf_max = buf_used + read_count;
        size_t buf_last_element = 0;

        for (i = buf_used; i < buf_max; i++)
        {
            if (buf[i] == delim)
            {
                buf[i] = 0;
                cmd_argv[cmd_argc] = buf + buf_last_element;
                pid_t pid = fork();
                if (pid)
                {
                    int status;
                    waitpid(pid, &status, 0);
                    buf[i] = delim;
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        write_all(STDOUT_FILENO, buf + buf_last_element, i + 1 - buf_last_element);
                    }
                    buf_last_element = i + 1;
                } else
                {
                    int fd = open("/dev/null", O_WRONLY);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(cmd_argv[0], cmd_argv);
                    _exit(6);
                }
            } else
            {
                if (buf_last_element == 0 && i == buf_size - 1)
                {
                    return 5;
                }
            }
        }

        memmove(buf, buf + buf_last_element, buf_size - buf_last_element);
        buf_used = buf_max - buf_last_element;
        if (eof_reached)
        {
            break;
        }
    }

    free(buf);
    return 0;
}

