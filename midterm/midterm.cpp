#include <unistd.h>
#include <algorithm>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <vector>

const std::string TMP_BASE = "/tmp/midterm_";
const int BUFFER_SIZE = 1024;
const char DELIMITER = ' ';

void * safe_alloc(int count)
{
    void * result = malloc(count);
    if (result == NULL)
        _exit(125);
    return result;
}

int safe_read(int fd, char * buffer, size_t count)
{
    int result = read(fd, buffer, count);

    if (result == -1)
    {
        std::cerr << "IO error\n";
        _exit(21);
    }

    return result;
}

int safe_write(int fd, char * buffer, size_t count)
{
    int result = write(fd, buffer, count);

    if (result == -1)
    {
        std::cerr << "IO error\n";
        _exit(21);
    }

    return result;
}

void write_all(int fd, char * buf, size_t count)
{
    size_t written;

    for (written = 0; written < count;)
    {
        written += safe_write(fd, buf + written, count - written);
    }
}

void copy_fd(int from, int to)
{
    const int bs = BUFFER_SIZE;
    char * buf = (char *) safe_alloc(bs * sizeof(char));
    int read_result = 0;

    while((read_result = safe_read(from, buf, bs)) > 0)
    {
        write_all(to, buf, read_result);
    }

    free(buf);
}

int safe_open(std::string const& path, int flags)
{
    int fd = open(path.c_str(), flags);

    if (fd == -1)
    {
        std::cerr << "IO error\n";
        _exit(20);
    }

    return fd;
}

int safe_open(std::string const& path, int flags, mode_t mode)
{
    int fd = open(path.c_str(), flags, mode);

    if (fd == -1)
    {
        std::cerr << "IO error\n";
        _exit(20);
    }

    return fd;
}


bool compare_files(int old_fd, int new_fd)
{
    char * old_buf = (char *) safe_alloc(BUFFER_SIZE * sizeof(char));
    char * new_buf = (char *) safe_alloc(BUFFER_SIZE * sizeof(char));

    int old_used = 0, new_used = 0;
    int pos = 0;
    bool old_eof = false;
    bool new_eof = false;

    bool equal = true;

    while(true)
    {
        for (; pos < old_used && pos < new_used; pos++)
        {
            if (old_buf[pos] != new_buf[pos])
            {
                equal = false;
                break;
            }
        }

        if (!equal)
        {
            break;
        }

        if (pos == BUFFER_SIZE)
        {
            pos = 0;
            old_used = 0;
            new_used = 0;
        }

        if (pos == old_used)
        {
            int read_result = safe_read(old_fd, old_buf + old_used, BUFFER_SIZE - old_used);
            old_eof = read_result == 0;
            old_used += read_result;
        }

        if (pos == new_used)
        {
            int read_result = safe_read(new_fd, new_buf + new_used, BUFFER_SIZE - new_used);
            new_eof = read_result == 0;
            new_used += read_result;
        }

        if (old_eof ^ new_eof)
        {
            equal = false;
            break;
        }
        else if (old_eof && new_eof)
        {
            break;
        }

    }

    free(old_buf);
    free(new_buf);
    return equal;
}

bool compare_files(std::string const& a, std::string const& b)
{
    int fd1 = safe_open(a, O_RDONLY);
    int fd2 = safe_open(b, O_RDONLY);

    bool result = compare_files(fd1, fd2);
    close(fd1);
    close(fd2);
    return result;
}

enum class state_t
{
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
          buffer((char *)safe_alloc((BUFFER_SIZE + 1) * sizeof(char)))
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
                    }
                    else if (buffer[i] == '\\')
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
                        _exit(6);
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
        if (eof)
        {
            return 0;
        }

        if (buf_used == BUFFER_SIZE)
        {
            return -1;
        }

        int read_result = safe_read(fd, buffer + buf_used, BUFFER_SIZE - buf_used);

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

void run_one(char ** args, int fd_in, int fd_out)
{
//    std::cout << "Running " << args[0] << "\n";
//    std::cout << "IN: " << fd_in << " OUT: " << fd_out << "\n";
    if (fd_in != STDIN_FILENO)
    {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (fd_out != STDOUT_FILENO)
    {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    execvp(args[0], args);
    _exit(8);
}

void run_all(std::vector<char **> const& commands, size_t pos, int fd_in, int fd_out)
{
    int pipefd[2];

    int pipe_in = fd_in;
    int pipe_out = fd_out;

//    std::cout << "Running command " << pos << "\n";

    if (pos != commands.size() - 1)
    {
        pipe(pipefd);
        pipe_in = pipefd[0];
        pipe_out = pipefd[1];
    }

    pid_t pid = fork();

    if (pid)
    {
        if (pos != commands.size() - 1)
        {
            close(pipe_out);
            run_all(commands, pos + 1, pipe_in, fd_out);
            close(pipe_in);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);

            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            {
                std::cerr << "Command with number " << pos << " finished with code " << WEXITSTATUS(status) << "\n";
                _exit(9);
            }
        }
    }
    else
    {
        run_one(commands[pos], fd_in, pipe_out);
    }
}


int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        std::cerr << "No file provided" << std::endl;
        return 1;
    }

    int fd = safe_open(argv[1], O_RDONLY);

    std::vector<char **> commands;
    {
        reader_t reader(fd);
        std::vector<std::string> current_command;

        while (reader.has_next())
        {
            std::string cur = reader.next_argument();

            if (cur == "|" || cur == "")
            {
                char ** cmd = (char **) safe_alloc((current_command.size() + 1) * sizeof(char *));
                cmd[current_command.size()] = NULL;


                for (size_t i = 0; i < current_command.size(); i++)
                {
                    cmd[i] = (char *) safe_alloc((current_command[i].size() + 1) * sizeof(char));
                    memcpy((void *) cmd[i], (const void *) current_command[i].c_str(), sizeof(char) * current_command[i].size());
                    cmd[i][current_command[i].size()] = '\0';

                    if (!reader.has_next() && i == current_command.size() - 1)
                    {
                        cmd[i][current_command[i].size() - 1] = '\0';
                    }
                }

                commands.push_back(cmd);
                current_command.clear();
            }
            else
            {
                current_command.push_back(cur);
            }
        }
    }

    for (int i = 0;; i++)
    {
        int in_fd, out_fd;
        std::string tmp_file = TMP_BASE + std::to_string(i);
        std::string tmp_old = i == 0 ? "" : TMP_BASE + std::to_string(i - 1);
        out_fd = safe_open(tmp_file, O_WRONLY | O_CREAT | O_TRUNC, 00644);

        in_fd = i == 0 ? STDIN_FILENO : safe_open(tmp_old, O_RDONLY);

        run_all(commands, 0, in_fd, out_fd);
        close(out_fd);

        if (i != 0)
        {
            close(in_fd);
        }

        if (i != 0 && compare_files(tmp_old, tmp_file))
        {
            int out = safe_open(tmp_file, O_RDONLY);
            copy_fd(out, STDOUT_FILENO);
            close(out);
            remove(tmp_file.c_str());
            remove(tmp_old.c_str());
            break;
        }

        remove(tmp_old.c_str());
    }

    for (auto command : commands)
    {
        for (int i = 0; command[i] != NULL; i++)
        {
            free(command[i]);
        }
        free(command);
    }
}

