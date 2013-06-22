#pragma once
#include "epollfd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>

class async_op
{
public:
    virtual ~async_op() {}
};

template<class Buf>
class async_read : public async_op
{
public:
    async_read(int fd, Buf& buf, epollfd& e, std::function<void()> const& cont, std::function<void()> const& cont_err) : fd(fd), e(e)
    {
        e.subscribe(fd, EPOLLIN, [&cont, this, &buf, fd] () { buf.read(fd); cont(); valid = false; }, 
                                [&cont_err, this] () { cont_err(); valid = false; });
    }

    async_read(async_read const&) = delete;
    async_read& operator= (async_read const&) = delete;

    async_read(async_read && other) : e(other.e), fd(other.fd), valid(other.valid)
    {
        other.valid = false;
    }

    ~async_read()
    {
        std::cout << "~async_read" << std::endl;
        if (valid)
        {
            e.unsubscribe(fd, EPOLLIN);
        }
    }
private:
    bool valid = true;
    int fd;
    epollfd& e;
};

template<class Buf>
class async_write : public  async_op
{
public:
    async_write(int fd, Buf& buf, epollfd& e, std::function<void()> const& cont, std::function<void()> const& cont_err) : fd(fd), e(e)
    {
        e.subscribe(fd, EPOLLOUT, [&cont, this, &buf, fd] () { buf.write(fd); cont(); valid = false; }, 
                                [&cont_err, this] () { cont_err(); valid = false; });
    }

    async_write(async_write const&) = delete;
    async_write& operator= (async_write const&) = delete;

    async_write(async_write && other) : e(other.e), fd(other.fd), valid(other.valid)
    {
        other.valid = false;
    }

    ~async_write()
    {
        std::cout << "~async_write" << std::endl;
        if (valid)
        {
            e.unsubscribe(fd, EPOLLOUT);
        }
    }
private:
    bool valid = true;
    int fd;
    epollfd& e;
};

class async_accept : public async_op
{
public:
    async_accept(int fd, epollfd& e, std::function<void(int)> const& cont, std::function<void()> const& cont_err) : fd(fd), e(e)
    {
        e.subscribe(fd, EPOLLIN, [&cont, this, fd] () { 
            sockaddr_in clientaddr;
            socklen_t addrlen = sizeof(clientaddr);
            int connection_fd = accept(fd, (sockaddr*) &clientaddr, &addrlen);
            cont(connection_fd);
            valid = false; 
        }, 
        [&cont_err, this] () { cont_err(); valid = false; });
    }

    async_accept(async_accept const&) = delete;
    async_accept& operator= (async_accept const&) = delete;

    async_accept(async_accept && other) : valid(other.valid), fd(other.fd), e(other.e)
    {
        other.valid = false;
    }

    ~async_accept()
    {
        if (valid)
        {
            e.unsubscribe(fd, EPOLLIN);
        }
    }
private:
    bool valid = true;
    int fd;
    epollfd& e;
};
