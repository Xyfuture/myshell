#include <stdio.h>

char test[2][3];
char* list[2];
int main()
{
    list[1]=test[0];
    printf("level 2:%p\n",test+1);
    printf("level 1:%p\n",test[1]);
    printf("level t:%p\n",list[1]);
    printf("size: %lu\n",sizeof(test));
    return 0;
}