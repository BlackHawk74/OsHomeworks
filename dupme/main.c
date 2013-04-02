#include <unistd.h>
#include <stdlib.h>

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
        output_start= 0;
        for (i = len; i < len + read_res; ++i)
        {
            if (buf[i] == '\n')
            {
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
                        write_res = write(STDOUT, buf + output_start, output_end - output_start);
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
        memmove(buf, buf + output_start, len + read_res - output_start + 1);
        len = len + read_res - output_start + 1;
    }
    return 0;
}

