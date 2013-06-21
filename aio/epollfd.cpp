#include "epollfd.h"

static const int MAX_EVENTS = 10;

epollfd::epollfd()
{
    efd = epoll_create(1);
    if (efd == -1)
    {
        throw std::runtime_error("could not create epoll");
    }
}

void epollfd::add_to_epoll(int fd, int what)
{
    epoll_event event;
    event.events = what;
    event.data.fd = fd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event))
    {
        throw std::runtime_error("could not add event to epoll");
    }
}

void epollfd::del_from_epoll(int fd)
{
    if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL))
    {
        throw std::runtime_error("could not delete event from epoll");
    }
}

void epollfd::mod_epoll(int fd)
{
    epoll_event event;
    event.events = flags[fd];
    event.data.fd = fd;

    if(epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event)) 
    {
        throw std::runtime_error("could not mod epoll");
    }
}


void epollfd::subscribe(int fd, int what, std::function<void()> const& cont, std::function<void()> const& cont_err)
{
    auto fd_row = data.find(fd);
    if (fd_row == data.end())
    {
        add_to_epoll(fd, what);
        std::unordered_map<int, std::pair<std::function<void()>, std::function<void()>>> new_row;
        new_row.insert(std::make_pair(what, std::make_pair(cont, cont_err)));
        data.insert(std::make_pair(fd, new_row));
        flags[fd] = what;
    } else 
    {
        auto& row = (*fd_row).second;
        auto what_row = row.find(what);
        if (what_row == row.end())
        {
            if (flags[fd] & what)
            {
                throw std::runtime_error("already signed to this event");
            }
            flags[fd] |= what;
            try {
                mod_epoll(fd);
            } catch (std::exception const&)
            {
                flags[fd] ^= what;
                throw;
            }
            row.insert(make_pair(what, make_pair(cont, cont_err)));
        } else
        {
            throw std::exception();
        }
        
    }
}

void epollfd::unsubscribe(int fd, int what)
{
    auto fd_row = data.find(fd);

    if (fd_row == data.end())
    {
        throw std::exception();
    } else
    {
        if ((*fd_row).second.find(what) == (*fd_row).second.end())
        {
            throw std::exception();
        }

        (*fd_row).second.erase(what);

        flags[fd] &= ~what;
        if ((*fd_row).second.empty())
        {
            try {
                del_from_epoll(fd);
            } catch (std::exception const&)
            {
                flags[fd] |= what;
                throw;
            }

            data.erase(fd_row);
            flags.erase(fd);
        } else 
        {
            try {
                mod_epoll(fd);
            } catch (std::exception const&)
            {
                flags[fd] |= what;
                throw;
            }
        }
    }
}

void epollfd::cycle()
{
    epoll_event events[MAX_EVENTS];

    int count = epoll_wait(efd, events, MAX_EVENTS, -1);

    if (count == -1)
    {
        throw std::exception();
    }

    for (int i = 0; i < count; i++)
    {
        epoll_event& event = events[i];
        auto& row = data[event.data.fd];
        auto it = row.begin();
        while(it != row.end())
        {

            auto ev = *it;

            auto next = std::next(it);
            row.erase(it);
            it = next;
            flags[event.data.fd] &= ~ev.first;
            try {
                mod_epoll(event.data.fd);
            } catch(std::exception const&)
            {
                flags[event.data.fd] |= ev.first;
                throw;
            }

            if ((event.events & (EPOLLERR | EPOLLHUP))
                && !(event.events & (EPOLLIN | EPOLLHUP)))
            {
                ev.second.second();
            } else if (ev.first & event.events)
            {
                ev.second.first();
            }
        }
        if (row.empty())
        {
            del_from_epoll(event.data.fd);
            data.erase(event.data.fd);
        }
    }
}

epollfd::~epollfd()
{
    close(efd);
}
