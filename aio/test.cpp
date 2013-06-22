#include "epollfd.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

void write_all(int fd, const char * buf, int size)
{
    for (int written = 0; written < size; )
    {
        int w = write(fd, buf + written, size - written);
        if (w < 0) throw std::exception();

        written += w;
    }
}

int main()
{
    epollfd e;

    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE];
    int buf_pos = 0;

    std::function<void()> err = [] () { std::cerr << "error"; };
    std::function<void()> func;
    func = [&e, &buf, &buf_pos, &func, &err] () -> void {

        int r = read(STDIN_FILENO, buf + buf_pos, BUF_SIZE - buf_pos);

        if (r > 0 && buf_pos < BUF_SIZE)
        {
            buf_pos += r;
            e.subscribe(STDIN_FILENO, EPOLLIN, func, err);
            e.cycle();
        } 
    };

    e.subscribe(STDIN_FILENO, EPOLLIN, func, err);
    e.cycle();
    write_all(STDOUT_FILENO, buf, buf_pos);
}
