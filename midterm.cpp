#include <unistd.h>
#include <cstring>
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
    _has_next(true),
    buffer((char*)malloc((BUFFER_SIZE + 1) * sizeof(char)))
    {}


    std::string next_argument()
    {
        std::string result = "";
        state_t state = state_t::NORMAL;
        
//        std::cout << "next_argument — eof: " << eof << " buf_start: " << buf_start << " buf_used: " << buf_used << "\n";
        while(true)
        {
            for (int i = buf_start; i < buf_used; i++)
            {
                switch (state)
                {
                case state_t::NORMAL:
                    if (buffer[i] == DELIMITER)
                    {
//                        std::cout << "Delimiter " << i << "\n";
                        result.append(buffer + buf_start, buffer + i);
                        buf_start = i + 1;
                        return result;
                    } else if (buffer[i] == '\\')
                    {
                        result.append(buffer + buf_start, buffer + i);
                        buf_start = i + 1;
                        state = state_t::ESCAPED;
                    }
                    break;
                case state_t::ESCAPED:
                    if (buffer[i] != '\\' && buffer[i] != DELIMITER)
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
                _has_next = false;
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

    bool has_next()
    {
        return _has_next;
    }

    ~reader_t()
    {
        free(buffer);
        close(fd);
    }

private:
    int fd;
    int buf_used;
    int buf_start;
    bool eof;
    bool _has_next;
    char * buffer;

    int do_read()
    {
//        std::cout << "do_read()\n";
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
    }
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

    std::vector<char**> commands;
    reader_t reader(fd);
    std::vector<std::string> current_command;

    while (reader.has_next())
    {
        std::string cur = reader.next_argument();
        if (cur == "|" || cur == "")
        {
            char** cmd = (char **) malloc((current_command.size() + 1) * sizeof(char*));
            cmd[current_command.size()] = NULL;
            for (int i = 0; i < current_command.size(); i++)
            {
                cmd[i] = (char*) malloc((current_command[i].size() + 1) * sizeof(char));
                memcpy((void*) cmd[i], (const void*) current_command[i].c_str(), sizeof(char) * current_command[i].size());
                cmd[i][current_command[i].size()] = '\0';
            }
            commands.push_back(cmd);
            current_command.clear();
        } else {
            current_command.push_back(cur);
        }
    }

    for (auto s: commands)
    {
        for (int i = 0; s[i] != NULL; i++)
        {
            std::cout << s[i] << " ";
        }
        std::cout << "\n--\n";

    }
}

