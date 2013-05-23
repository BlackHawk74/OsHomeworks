#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

const char* FIFO_OLD = "/tmp/watchthis_fifo0";
const char* FIFO_NEW = "/tmp/watchthis_fifo1";

void * safe_alloc(int count)
{
    void * result = malloc(count);
    if (result == NULL)
        _exit(125);
    return result;
}

void remove_fifos()
{
    remove(FIFO_OLD);
    remove(FIFO_NEW);
}

void exit_function()
{
    remove_fifos();
    _exit(0);
}

void write_all(int fd, char * buf, int count)
{
    int written;
    for (written = 0; written < count; )
    {
        int write_res = write(fd, buf + written, count - written);
        if (write_res < 0) 
        {
            _exit(10);
        }
        written += write_res;
    }
}

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

    int fifo0 = mkfifo(FIFO_OLD, S_IRUSR | S_IWUSR);
    int fifo1 = mkfifo(FIFO_NEW, S_IRUSR | S_IWUSR);
    if (fifo0 || fifo1)
    {
        remove_fifos();
        return 3;
    }
    signal(SIGINT, exit_function); 
    signal(SIGTERM, exit_function);

    int pipefd[2];

    size_t buf_size = 100;
    size_t used = 0;
    char * buf = (char*) safe_alloc(buf_size);

    char * old_buf = (char*) safe_alloc(buf_size);
    size_t old_used = 0;

    while (1) {
        pipe(pipefd);
        if (fork())
        {
            close(pipefd[1]);
            wait(NULL);

            used = 0;
            int read_result;
            while((read_result = read(pipefd[0], buf + used, buf_size - used)) > 0)
            {
//                write(1, "read\n", 5);
                used += read_result;
                if (used == buf_size)
                {
//                    write(1, "realloc\n", 8);
                    buf_size *= 2;
                    buf = (char*) realloc((void*) buf, buf_size);
                    old_buf = (char*) realloc((void*) old_buf, buf_size);
                }
            }
            close(pipefd[0]);

            write_all(STDOUT_FILENO, buf, used);

            if (old_used > 0)
            {

                if (fork())
                {
                    int fd0 = open(FIFO_OLD, O_WRONLY);
                    int fd1 = open(FIFO_NEW, O_WRONLY);

                    
                    write_all(fd0, old_buf, old_used);
                    write_all(fd1, buf, used);

                    close(fd0);
                    close(fd1);
                    
                    wait(NULL);
                    
                } else {
                    execlp("diff", "diff", "-u", FIFO_OLD, FIFO_NEW, NULL);
                }
            }
            
            char * t = old_buf;
            old_buf = buf;
            buf = t;
            old_used = used;
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

