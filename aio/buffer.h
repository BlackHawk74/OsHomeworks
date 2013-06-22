#pragma once
#include <cstring>
#include <stdexcept>
#include <unistd.h>


class buffer
{
public:
    buffer(int size) : size(size), end(0), buf((char*) malloc(size * sizeof(char)))
    {
        if (buf == nullptr)
        {
            throw std::runtime_error("Could not allocate buffer");
        }
    }

    bool read(int fd)
    {
        if (fd < 0)
            throw std::invalid_argument("fd < 0");

        if (size == end)
            throw std::runtime_error("buffer is full");

        int r = ::read(fd, buf + end, size - end);
        if (r < 0)
            throw std::runtime_error("could not read from file descriptor");
        
        if (r == 0)
        {
            return false;
        }

        end += r;
        return true;
    }

    bool write(int fd)
    {
        if (fd < 0)
            throw std::invalid_argument("fd < 0");

        int w = ::write(fd, buf, end);

        if (w < 0)
            throw std::runtime_error("could not write to file descriptor");

        if (w == 0)
            return false;

        memmove(buf, buf + w, end - w);
        end -= w;
        return true;
    }

    ~buffer()
    {
        free(buf);
    }
private:
    int size, end; 
    char * buf;
};
