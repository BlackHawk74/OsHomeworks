#include "epollfd.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>


int main()
{
    epollfd e;

    e.subscribe(STDIN_FILENO, EPOLLIN, [&e] () {
        std::cout << "success\n";
        char buf[4096];
        int r = read(STDIN_FILENO, buf, 4096);
        
        e.subscribe(STDOUT_FILENO, EPOLLOUT, [r, buf] () {
            write(STDOUT_FILENO, buf, r);
        }, [] () { std::cout << "err2\n";});
        }, [] () { std::cout << "error\n"; });
    e.cycle();
    e.cycle();
}
