#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void write_all(int fd, char * buf, int count)
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

int main(int argc, char** argv)
{
    // Default values
    int buf_size = 4096;
    char delim = '\n';

    int opt_count = 1;
    
    // Parsing options
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
                write_all(STDERR_FILENO, "Buffer size should be positive number\n", 39);
                _exit(2);
            }
            opt_count++;
            break;
        }
        opt_count++;
    }

    if (argc > opt_count
        && strcmp(argv[opt_count], "--") == 0) // "--" was used to mark end of options
    {
        opt_count++;
    }
    
    if (opt_count >= argc)
    {
        write_all(STDERR_FILENO, "No command provided\n", 21);
        _exit(1);
    }

    // One more char for delimiter
    buf_size++;
    char * buf = malloc(sizeof(char) * buf_size);

    // Creating command to execute
    char** cmd_argv = malloc(sizeof(char*) * (argc - opt_count + 2));
    int cmd_argc = argc - opt_count;
    memcpy(cmd_argv, argv + opt_count, sizeof(char*) * cmd_argc);
    cmd_argv[cmd_argc + 1] = NULL;

    int eof_reached = 0;
    int buf_used = 0;

    while (1)
    {
        int read_count = read(STDIN_FILENO, buf + buf_used, buf_size - buf_used);
        
        if (read_count < 0) // IO error
        {
            _exit(4);
        }

        if (!read_count) // EOF found
        {
            eof_reached = 1;
            if (buf_used == 0)
            {
                break;
            }

            // There is the last line in buffer
            buf[buf_used] = delim;
            read_count++;
        }

        int i;
        int buf_max = buf_used + read_count;
        int buf_last_element = 0;

        for (i = buf_used; i < buf_max; i++)
        {
            if (buf[i] == delim) // Delimiter found
            {
                // Passing argument to command
                // As the last character set to 0, we don't need to care what symbols are after it
                buf[i] = 0;
                cmd_argv[cmd_argc] = buf + buf_last_element;

                pid_t pid = fork();

                if (pid) // parent
                {
                    int status;

                    // Waiting for child to exit
                    waitpid(pid, &status, 0);
                    buf[i] = delim;

                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                    {
                        // Command executed with exit code 0
                        write_all(STDOUT_FILENO, buf + buf_last_element, i + 1 - buf_last_element);
                    }
                    buf_last_element = i + 1;
                } else // child
                {
                    // Redirecting output
                    int fd = open("/dev/null", O_WRONLY);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);

                    // Executing command
                    execvp(cmd_argv[0], cmd_argv);
                    _exit(6); // Something went wrong
                }
            } else
            {
                if (i == buf_size - 1 && buf_last_element == 0)
                {
                    // Buffer overflow
                    _exit(5);
                }
            }
        }

        // Buffer shifting
        memmove(buf, buf + buf_last_element, buf_size - buf_last_element);
        buf_used = buf_max - buf_last_element;

        if (eof_reached)
        {
            break;
        }
    }

    // Releasing allocated memory
    free(cmd_argv);
    free(buf);

    _exit(0);
}

