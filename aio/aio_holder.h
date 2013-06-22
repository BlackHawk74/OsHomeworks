#pragma once
#include "async_operations.h"
#include <memory>
#include <unordered_map>

class aio_holder
{
public:
    aio_holder(epollfd& e) : e(e), max_id(0)
    {}

    template<class Buf>
    void aread(int fd, Buf buf, std::function<void()> const& cont, std::function<void()> const& cont_err)
    {
        int id = max_id++;
        data.insert(std::make_pair(id, std::unique_ptr<async_op>(new async_read<Buf>(fd, buf, e, 
                    [&cont, id, this] () { this->data.erase(id); cont(); }, 
                    [&cont_err, this, id] () { this->data.erase(id); cont_err(); }))));
    }

    template<class Buf>
    void awrite(int fd, Buf buf, std::function<void()> const& cont, std::function<void()> const& cont_err)
    {
        int id = max_id++;
        data.insert(std::make_pair(id, std::unique_ptr<async_op>(new async_write<Buf>(fd, buf, e, 
                    [&cont, id, this] () { this->data.erase(id); cont(); }, 
                    [&cont_err, this, id] () { this->data.erase(id); cont_err(); }))));
    }

    void aaccept(int fd, std::function<void(int)> const& cont, std::function<void()> const& cont_err)
    {
        int id = max_id++;
        data.insert(std::make_pair(id, std::unique_ptr<async_op>(new async_accept(fd, e, 
                    [&cont, id, this] (int fd) { data.erase(id); cont(fd); }, 
                    [&cont_err, this, id] () { data.erase(id); cont_err(); }))));
    }
private:
    std::unordered_map<int, std::unique_ptr<async_op>> data;
    epollfd& e;
    int max_id;
};
