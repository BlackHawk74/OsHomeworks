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

    while ((read_res = read(STDIN, buffer + len, k - len)) > 0)
    {
        printf("read_res = %d\n", read_res);
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
                        write_res = write(STDOUT, buffer + output_start, output_end - output_start);
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
        printf("current output start %d\n", output_start);
        memmove(buffer, buffer + output_start, len + read_res - output_start);
        len = len + read_res - output_start;
        if (len == k)
        {
            current_state = IGNORING;
            len = 0;
        }
    }

    return 0;
}

