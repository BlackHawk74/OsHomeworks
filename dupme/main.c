#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define STDIN 0
#define STDOUT 1


enum state {
    NORMAL, IGNORING
} current_state;

int main(int argc, char** argv)
{
    int k;
    char *buffer;
    int len;
    int read_res;
    int output_start, output_end;
    int i, j;
    int write_start;
    int write_res;
    int eof_reached;

    if (argc < 2)
    {
        return 1;
    }

    k = atoi(argv[1]);

    if (k < 1)
    {
        return 2;
    }

    k++;
    buffer = (char *) malloc(k + 1);
    len = 0;
    current_state = NORMAL;
    eof_reached = 0;

    while (1)
    {
        read_res = read(STDIN, buffer + len, k - len);
        printf("read_res = %d\n", read_res);
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
        output_start= 0;
        for (i = len; i < len + read_res; ++i)
        {
            if (buffer[i] == '\n')
            {
                printf("found newline at %d\n", i);
                if (current_state == IGNORING)
                {
                    output_start = i + 1;
                    current_state = NORMAL;
                }
                output_end = i + 1;
                for (j = 0; j < 2; ++j)
                {
                    write_start = output_start;
                    while (write_start < output_end)
                    {
                        write_res = write(STDOUT, buffer + write_start, output_end - output_start);
                        if (write_res == -1)
                        {
                            return 3;
                        }
                        write_start += write_res;
                    }
                }
                output_start = i + 1;
            }
        }
        if (eof_reached)
        {
            break;
        }
        printf("current output start %d\n", output_start);
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

