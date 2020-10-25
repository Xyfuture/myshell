#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#define PIPE_COUNT 128
#define COMMAND_BUF 256 // per command character size   一个参数的长度
#define COMMAND_NUM 128 // command parameters          多少个参数
#define COMMAND_COUNT 128 // command count            总共多少条命令

typedef struct commandInfo
{
    int startPosi;
    int endPosi;
    int num;//which one
    int pipeIn;//pipe number
    int pipeOut;//pipe number
} commandInfo;


int pipeNum;//total pipe number
int commandNum;// total command number

char commandInput[COMMAND_BUF*COMMAND_COUNT*COMMAND_NUM];//input
char commands[COMMAND_NUM][COMMAND_BUF];
char *commandsPara[COMMAND_NUM]; // for execvp
int allPipes[PIPE_COUNT][2];








int main(int argc,char ** argv)
{
    
    return 0;
}