#include "buffer.h"
#include "async_operations.h"
#include <iostream>
#include "aio_holder.h"

int main()
{
    buffer buf(4096);
    epollfd e;

    aio_holder holder(e);

    holder.aread(STDIN_FILENO, buf, [&e, &buf, &holder] () {
        std::cout << "success\n";
        holder.awrite(STDOUT_FILENO, buf, [] () { std::cout << "success\n"; }, [] () { std::cout << "error\n"; });
        e.cycle();
    }, [] () { std::cout << "error\n"; });

    //async_read<buffer> ar(STDIN_FILENO, buf, e, [&e, &buf] () { 
        //std::cout << "success\n";
        //async_write<buffer> aw(STDOUT_FILENO, buf, e, [] () { std::cout << "success\n"; }, [] () { std::cout << "error\n"; });
        //e.cycle();
    //}, [] () { std::cout << "error\n"; });
    e.cycle();

    while(buf.write(STDOUT_FILENO));
}
