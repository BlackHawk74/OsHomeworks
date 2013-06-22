#include <stdexcept>
#include <unistd.h>
#include <functional>
#include <unordered_map>
#include <sys/epoll.h>

class epollfd
{
public:
    epollfd();

    epollfd(epollfd const&) = delete;
    epollfd& operator= (epollfd const&) = delete;

    void subscribe(int fd, int what, std::function<void()> const& cont, std::function<void()> const& cont_err);
    void unsubscribe(int fd, int what);
    
    void cycle();

    ~epollfd();

private:
    bool add_to_epoll(int fd);
    bool del_from_epoll(int fd);
    bool mod_epoll(int fd);

    int efd;

    std::unordered_map<int, 
        std::unordered_map<int, 
            std::pair<
                std::function<void()>, 
                std::function<void()>
            >
        >
    > data;

    std::unordered_map<int, int> flags;
};
