#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <vector>

const int BUFFER_SIZE = 1024;
const char DELIMITER = ' ';

enum class state_t {
    NORMAL, ESCAPED
};

class reader_t
{
public:
    reader_t(int fd) 
    : fd(fd),
    buf_used(0),
    buf_start(0),
    eof(false),
    buffer((char*)malloc((BUFFER_SIZE + 1) * sizeof(char)))
    {}


    std::string next_argument()
    {
        std::string result = "";
        int read_result = 0;
        state_t state = state_t::NORMAL;
        
        while(true)
        {
            for (int i = buf_start; i < buf_used; i++)
            {
                switch (state)
                {
                case state_t::NORMAL:
                    if (buffer[i] == DELIMITER)
                    {
                        result.append(buffer + buf_start, buffer + i);
                        buf_start = i + 1;
                        return result;
                    } else if (buffer[i] == '\')
                    {
                        result.append(buffer + buf_start, buffer + i);
                        buf_start = i + 1;
                        state = state_t::ESCAPED;
                    }
                    break;
                case state_t::ESCAPED:
                    if (buffer[i] != '\' && buffer[i] != DELIMITER)
                    {
                        std::cerr << "Error in input file" << std::endl;
                        _exit(4);
                    }
                    state = state_t::NORMAL;
                    break;

                }
            }

            if (eof)
            {
                break;
            }

            if (do_read() == -1)
            {
                result.append(buffer + buf_start, buffer + buf_used);
                buf_start = 0;
                buf_used = 0;
                do_read();
            }

        }
        return result;

    }

    ~reader_t()
    {
        free(buffer);
    }

private:
    int fd;
    int buf_start;
    int buf_used;
    bool eof;
    char * buffer;

    int do_read()
    {
        if (eof) return 0;
        if (buf_used == BUFFER_SIZE)
        {
            return -1;
        }

        int read_result = read(fd, buffer + buf_used, BUFFER_SIZE - buf_used);
        if (read_result < 0)
        {
            std::cerr << "IO error" << std::endl;
            _exit(3);
        }
        if (read_result == 0)
        {
            read_result++;
            buffer[buf_used] = DELIMITER;
            eof = true;
        }

        buf_used += read_result;
        return read_result;

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

