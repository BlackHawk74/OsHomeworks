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
    char * buf;
    if (argc < 2)
    {
        puts("No command provided");
        return 1;
    }
    int opt_count = 1;
    char delim_c = '\n';
    while (1)
    {
        int next_opt = getopt(argc, argv, "nzb:");
        if (next_opt == -1)
        {
            break;
        }
        switch (next_opt)
        {
        case 'n':
            delim_c = '\n';
            break;
        case 'z':
            delim_c = 0;
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

    opt_count++;

    buf_size++;
    buf = malloc(buf_size);

    size_t buf_used = 0;
    int read_count;

    char ** prog_argv = malloc(sizeof(char*) * (argc - opt_count + 2));
//    char * prog_last;
    int j, prog_c = 0;
    for (j = opt_count; j < argc; j++)
    {
        prog_argv[prog_c++] = argv[j];
    }
    prog_argv[prog_c] = buf;
    prog_argv[prog_c + 1] = NULL;


    int last_input = 0;

    while (1)
    {
        read_count = read(STDIN_FILENO, buf + buf_used, buf_size - buf_used);
        if (!read_count) {
            last_input = 1;
            if (buf_used == 0)
            {
                break;
            }
            buf[buf_used] = delim_c;
            read_count++;
        }
        size_t i;
        size_t buf_max = buf_used + read_count;
        for (i = buf_used; i < buf_max; i++)
        {
            if (buf[i] == delim_c)
            {
                buf[i] = 0;
                pid_t pid = fork();
                if (pid)
                {
                    int status;
                    waitpid(pid, &status, 0);
                    buf[i] = delim_c;
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        write_all(STDOUT_FILENO, buf, i + 1);
                    }
                    memmove(buf, buf + i + 1, buf_size - i - 1);
                    buf_max -= i + 1;
                    i = 0;
                } else 
                {
                    int fd = open("/dev/null", O_WRONLY);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(prog_argv[0], prog_argv);
                    _exit(6);
                }
            } else 
            {
                if (i == buf_size - 1)
                {
                    return 5;
                }
            }
        }
        buf_used = buf_max;
        if (last_input)
        {
            break;
        }
    }

    return 0;
}

