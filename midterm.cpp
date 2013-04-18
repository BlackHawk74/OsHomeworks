#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <vector>

const int BUFFER_SIZE = 1024;

enum class state_t {
    NORMAL, ESCAPED
};

class reader_t
{
public:
    reader_t(int fd) 
    : fd(fd),
    buf_used(0),
    buffer((char*)malloc(BUFFER_SIZE * sizeof(char)))
    {}


    std::string next_argument()
    {
        std::string result = "";
        int read_result = 0;
        state_t state = state_t::NORMAL;
        
        while(1)
        {
            read_result = read(fd, buffer + buf_used, BUFFER_SIZE - buf_used);

            int end = buf_used + read_result;
        }

        return result;

    }

    ~reader_t()
    {
        free(buffer);
    }

private:
    int fd;
    int buf_used;
    char * buffer;
};


int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        std::cerr << "No file provided" << std::endl;
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1)
    {
        std::cerr << "Unable to open file";
        return 2;
    }

    std::vector<std::string> command;
    reader_t reader(fd);

}

