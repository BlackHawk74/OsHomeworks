#include <unistd.h>
#include <stdlib.h>

int k;
char *buffer;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        return 1;
    }

    k = atoi(argv[1]);

    if (k < 1)
    {
        return 2;
    }

    buffer = (char *) malloc(k);

    return 0;
}

