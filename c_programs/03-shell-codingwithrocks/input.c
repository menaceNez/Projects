#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char buf[128];
    fgets(buf, 128, stdin);
    printf("input.c InputBuffer: %s\n", buf);
    return 0;
}