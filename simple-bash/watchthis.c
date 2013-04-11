#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        puts("Usage: watchthis INTERVAL COMMAND");
        return 1;
    }
    int interval = atoi(argv[1]);
    if (interval <= 0)
    {
        puts("Interval must be greater than zero");
        return 2;
    }

    while (true) {
        sleep(interval);
    }
    return 0;
}

