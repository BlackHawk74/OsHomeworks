#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define STDIN 0
#define STDOUT 1


typedef enum state {
    NORMAL, IGNORING
} state;

void write_twice(char * buffer, int output_start, int output_end) 
{
    int i;
    for (i = 0; i < 2; i++)
    {
        int write_start = output_start;
        while (write_start < output_end)
        {
            int write_res = write(STDOUT, buffer + write_start, output_end - write_start);
            if (write_res == -1)
            {
                _exit(3);
            }
            write_start += write_res;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        return 1;
    }

    int k = atoi(argv[1]);

    if (k < 1)
    {
        return 2;
    }

    k++;
    char * buffer = (char *) malloc(k);
    int len = 0;
    state current_state = NORMAL;
    int eof_reached = 0;

    while (1)
    {
        int read_res = read(STDIN, buffer + len, k - len);
        //printf("read_res = %d\n", read_res);
        if (read_res == 0) {
            // EOF
            if (len > 0 && current_state == NORMAL)
            {
                buffer[len] = '\n';
                read_res = len + 1;
                len = 0;
            }
            eof_reached = 1;
        }
        else if (read_res < 0)
        {
            // Some read error
            return 4;
        }

        int i;
        int output_start= 0;
        int output_end;

        for (i = len; i < len + read_res; ++i)
        {
            if (buffer[i] == '\n')
            {
                //printf("found newline at %d\n", i);
                if (current_state == IGNORING)
                {
                    output_start = i + 1;
                    current_state = NORMAL;
                }
                output_end = i + 1;
                write_twice(buffer, output_start, output_end);
                output_start = i + 1;
            }
        }
        if (eof_reached)
        {
            break;
        }
        //printf("current output start %d\n", output_start);
        memmove(buffer, buffer + output_start, len + read_res - output_start);
        len = len + read_res - output_start;
        if (len == k)
        {
            current_state = IGNORING;
            len = 0;
        }
    }
    free(buffer);
    return 0;
}

