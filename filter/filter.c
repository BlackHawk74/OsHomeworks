#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


const int DELIM_ZERO = 0, DELIM_NEWLINE = 1;

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
    int delim = DELIM_NEWLINE;

    while (1)
    {
        int next_opt = getopt(argc, argv, "nzb:");
//        printf("%d\n", next_opt);
        if (next_opt == -1)
        {
            break;
        }
//        char *opt = argv[opt_count + next_opt];
        switch (next_opt)
        {
        case 'n':
            delim = DELIM_NEWLINE;
//            puts("NEWLINE");
            break;
        case 'z':
            delim = DELIM_ZERO;
//            puts("ZERO");
            break;
        case 'b':
            buf_size = atoi(optarg);
            if (buf_size <= 0)
            {
                return 2;
            }
//            printf("BUF %d\n", buf_size);
            opt_count++;
            break;
        }
        opt_count++;
    }

    opt_count++;

//   printf("%d\n", opt_count);

    buf_size++;
    buf = malloc(buf_size);

    size_t buf_used = 0;
//    size_t buf_pos = 0;
    int read_count;

    char delim_c = delim == DELIM_ZERO ? 0 : '\n';
    char ** prog_argv = malloc(sizeof(char*) * (argc - opt_count + 2));
    char * prog_last;
    int j, prog_c = 0;
    for (j = opt_count; j < argc; j++)
    {
        prog_argv[prog_c++] = argv[j];
    }
//    prog_argv[argc - opt_count];
    prog_argv[prog_c + 1] = NULL;


    int last_input = 0;

    while (1)
    {
        read_count = read(STDIN_FILENO, buf + buf_used, buf_size - buf_used);
//        printf("read %d\n", read_count);
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
//           printf("i %d\n", i);
            if (buf[i] == delim_c)
            {
                prog_last = malloc(i + 1);
                prog_last[i] = 0;
                prog_argv[prog_c] = prog_last;
                memcpy(prog_last, buf, i);
                pid_t pid = fork();
                if (pid)
                {
                    int status;
                    waitpid(pid, &status, 0);
//                   printf("%d\n", status);
                    free(prog_last);
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        int written;
                        for (written = 0; written <= i;)
                        {
                            written += write(STDOUT_FILENO, buf + written, i + 1 - written);
                        }
                    }
//                    printf("written buf");
                    memmove(buf, buf + i + 1, buf_size - i - 1);
                    buf_max -= i + 1;
                    i = 0;
//                   printf("%d %d\n", i, buf_max);
                } else 
                {
//                    write(1, "arg\n", 4);
//                   write(1, prog_last, i);
//                   write(1, "\n", 1);
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

