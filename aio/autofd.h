#include <unistd.h>
#include <stdexcept>

class autofd
{
    int fd;

public:
    autofd(int fd) : fd(fd)
    {
        if (fd < 0)
        {
            throw std::runtime_error("fd < 0");
        }
    }

    autofd(autofd const&) = delete;
    autofd& operator=(autofd const&) = delete;

    autofd(autofd && other) : fd(other.fd)
    {
        other.fd = -1;
    }

    autofd& operator= (autofd && other)
    {
        close(fd);
        fd = other.fd;
        other.fd = -1;
        return *this;
    }

    int get_fd()
    {
        return fd;
    }


    ~autofd()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }
};
