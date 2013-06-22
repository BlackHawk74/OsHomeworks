#include "epollfd.h"
#include <cstring>
#include <iostream>
#include <errno.h>

static const int MAX_EVENTS = 10;

epollfd::epollfd()
{
    efd = epoll_create(1);
    if (efd == -1)
    {
        throw std::runtime_error("could not create epoll");
    }
}

bool epollfd::add_to_epoll(int fd)
{
    std::cout << "adding to epoll " << fd << std::endl;
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = flags[fd];
    event.data.fd = fd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        return false;
    }
    return true;
}

bool epollfd::del_from_epoll(int fd)
{
    std::cout << "deleting from epoll " << fd << std::endl;
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL))
    {
        return false;
    }
    return true;
}

bool epollfd::mod_epoll(int fd)
{
    std::cout << "modifying epoll " << fd << std::endl;
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = flags[fd];
    event.data.fd = fd;

    if(epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event)) 
    {
        return false;
    }
    return true;
}


void epollfd::subscribe(int fd, int what, std::function<void()> const& cont, std::function<void()> const& cont_err)
{
    std::cout << "subscribe" << std::endl;
    for (auto x : data)
    {
        std::cout << x.first << std::endl;
    }
    std::cout << "------\n";

    auto fd_row = data.find(fd);
    if (fd_row == data.end())
    {
        flags[fd] = what;
        if (!add_to_epoll(fd))
        {
            flags.erase(fd);
            throw std::runtime_error("could not add to epoll");
        }
        //std::unordered_map<int, std::pair<std::function<void()>, std::function<void()>>> new_row;
        //new_row.insert(std::make_pair(what, std::make_pair(cont, cont_err)));
        //data.insert(std::make_pair(fd, new_row));
        data[fd].insert(std::make_pair(what, std::make_pair(cont, cont_err)));
    } else 
    {
        auto& row = (*fd_row).second;
        auto what_row = row.find(what);
        if (what_row == row.end())
        {
            if (flags[fd] & what)
            {
                throw std::invalid_argument("already subscribed to this event");
            }
            flags[fd] |= what;
            if (!mod_epoll(fd))
            {
                flags[fd] ^= what;
                throw std::runtime_error("could not modify epoll");
            }
            row.insert(make_pair(what, make_pair(cont, cont_err)));
        } else
        {
            throw std::runtime_error("already subscribed to this event");
        }
    }
}

void epollfd::unsubscribe(int fd, int what)
{
    std::cout << "unsubscribe" << std::endl;
    auto fd_row = data.find(fd);

    if (fd_row == data.end())
    {
        throw std::runtime_error("No such file descripor in epoll");
    } else
    {
        if ((*fd_row).second.find(what) == (*fd_row).second.end())
        {
            throw std::runtime_error("No such event for file descriptor");
        }

        (*fd_row).second.erase(what);

        flags[fd] &= ~what;
        if ((*fd_row).second.empty())
        {
            if (!del_from_epoll(fd))
            {
                flags[fd] |= what;
                throw std::runtime_error("Could not delete from epoll");
            }

            data.erase(fd_row);
            flags.erase(fd);
        } else 
        {
            if (!mod_epoll(fd))
            {
                flags[fd] |= what;
                throw std::runtime_error("could not modify epoll");
            }
        }
    }
}

void epollfd::cycle()
{
    std::cout << "cycle" << std::endl;
    epoll_event events[MAX_EVENTS];

    int count = epoll_wait(efd, events, MAX_EVENTS, -1);

    if (count == -1)
    {
        throw std::runtime_error("epoll_wait() error, code: " + std::to_string(errno));
    }

    for (int i = 0; i < count; i++)
    {
        auto& event = events[i];
        auto row = data.find(event.data.fd);
        auto it = row->second.begin();
        while(it != row->second.end())
        {
            std::cout << "starting handling events for " << event.data.fd << std::endl;
            auto ev = *it;

            flags[event.data.fd] &= ~ev.first;
            if (!mod_epoll(event.data.fd))
            {
                flags[event.data.fd] |= ev.first;
                throw std::runtime_error("could not modify epoll");
            }

            auto next = std::next(it);
            row->second.erase(it);
            it = next;

            bool is_empty = row->second.empty();

            if (is_empty)
            {
                if (!del_from_epoll(event.data.fd))
                {
                    throw std::runtime_error("could not delete from epoll");
                }
                data.erase(row);
                flags.erase(event.data.fd);
            }

            if ((event.events & (EPOLLERR | EPOLLHUP))
                && !(event.events & (EPOLLIN | EPOLLHUP))) // We may be able to read something from hupped descriptor
            {
                ev.second.second();
            } else if (ev.first & event.events)
            {
                ev.second.first();
            }

            if (is_empty) break;
        }
    }
}

epollfd::~epollfd()
{
    close(efd);
}
