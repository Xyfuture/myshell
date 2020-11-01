#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#define PIPE_COUNT 128
#define COMMAND_BUF 256 // per command character size   一个参数的长度
#define COMMAND_NUM 128 // command parameters          多少个参数
#define COMMAND_COUNT 128 // command count            总共多少条命令

typedef struct commandInfo
{
    int startPosi;
    int endPosi;
    //int num;//which one
    int pipeIn;//pipe number
    int pipeOut;//pipe number
    int reIn;
    int reOut;
} commandInfo;

int pipeNum;//total pipe number
int commandNum;// total command number
commandInfo commandTable[COMMAND_COUNT];

char commandInput[COMMAND_BUF*COMMAND_COUNT*COMMAND_NUM];//input
char commands[COMMAND_NUM][COMMAND_BUF];
char *commandsPara[COMMAND_NUM]; // for execvp every commmand generate before execute
char reInFile[COMMAND_BUF];
char reOutFile[COMMAND_BUF];
int allPipes[PIPE_COUNT][2];
pid_t childPid;

void sigcat()
{
    if(childPid == 0)
        return ;
    else if (childPid > 0)
        kill(childPid,SIGINT);
}

void praser()
{
    commandNum =0;
    pipeNum = 0;
    int cursor = 0;
    int len = strlen(commandInput);
    commandTable[commandNum].startPosi = 0;
    for(cursor=0;cursor<len;cursor++)
    {
        if(commandInput[cursor]=='|')
        {
            commandTable[commandNum].endPosi = cursor-1;
            commandTable[commandNum].pipeOut = pipeNum;// STDOUT -> pipeOut
            commandTable[++commandNum].pipeIn = pipeNum++;// next command STDIN -> pipeIn
            commandTable[commandNum].startPosi = cursor+1;
        }
    }
    commandTable[commandNum].endPosi = cursor-1;
    commandNum++;// totoal commands in one input
}

void initCommand() // init array
{
    commandInfo temp = {-1,-1,-1,-1,-1,-1};
    for(int i=0;i<COMMAND_COUNT;i++)
        commandTable[i] = temp;
    memset(commandInput,'\0',sizeof(commandInput));
    memset(commands,0,sizeof(commands));
    for(int i=0;i<PIPE_COUNT;i++)
        allPipes[i][0]=-1,allPipes[i][1]=-1;
    memset(reInFile,0,sizeof(reInFile));
    memset(reOutFile,0,sizeof(reOutFile));
    memset(commandsPara,0,sizeof(commandsPara));
    childPid = 0;
}

int redirect(int s,int e,int type)//parse redirection word
{
    int j=0;
    while(commandInput[s]==' ')
        s++;
    while(s<=e)
    {
        if(commandInput[s]==' ' || commandInput[s] == '<' || commandInput[s] == '>')
            break;
        if(type == 0)
            reInFile[j++] =commandInput[s++];
        else if (type==1)
            reOutFile[j++] = commandInput[s++];
    }
    return s;// the end of file name +1
}

void trans(int cur) // in one command (don't have pipe) find its redirection and args
{
    int s = commandTable[cur].startPosi;
    int e = commandTable[cur].endPosi;
    int cnt = 0,j=0,t=0;
    while(commandInput[s]==' ')// squeeze space
        s++;
    while(commandInput[e]==' ')
        e--;
    for(int i=s;i<=e;i++)
    {
        if(commandInput[i]==' ')//record next args
        {
            while(commandInput[i]==' '&&i<=e)
                i++;
            i--;
            if(j==0) // for some special case
                continue;
            commands[cnt][j] = '\0'; // finish last args
            commandsPara[t++] = commands[cnt];
            j=0;
            cnt++;// number of args
        }
        else if(commandInput[i]=='<')
        {
            i = redirect(i+1,e,0);
            i--;
            commandTable[cur].reIn = 1;
           
        }   
        else if(commandInput[i]=='>')
        {
            i = redirect(i+1,e,1);
            i--;
            commandTable[cur].reOut = 1;
        }
        else
            commands[cnt][j++] = commandInput[i];
    }
    if(j!=0)
    {
        commands[cnt][j] = '\0';
        commandsPara[t++] = commands[cnt];
    }
    commandsPara[t] = NULL;
}

void openFilesAndPipes(int cur) // open files and pipes change variable to file descriptor
{
    if(commandTable[cur].pipeOut!=-1) // open pipe 
        if (allPipes[commandTable[cur].pipeOut][1]==-1)
            pipe(allPipes[commandTable[cur].pipeOut]);
    if(commandTable[cur].reIn!=-1)
        commandTable[cur].reIn = open(reInFile,O_RDONLY);
    if(commandTable[cur].reOut!=-1)
        commandTable[cur].reOut = open(reOutFile,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
    if(commandTable[cur].pipeIn != -1)
        commandTable[cur].pipeIn = allPipes[commandTable[cur].pipeIn][0];
    if(commandTable[cur].pipeOut != -1)
        commandTable[cur].pipeOut = allPipes[commandTable[cur].pipeOut][1];
}

void closeFilesAndPipes(int cur) // close all in child and parent process
{
    if(commandTable[cur].reIn!=-1)
        close(commandTable[cur].reIn);
    if(commandTable[cur].reOut!=-1)
        close(commandTable[cur].reOut);
    if(commandTable[cur].pipeIn != -1)
        close(commandTable[cur].pipeIn);
    if(commandTable[cur].pipeOut != -1)
        close(commandTable[cur].pipeOut);
}

void changeIO(int cur)// in child process
{
    int in=commandTable[cur].reIn;
    int out=commandTable[cur].reOut;
    if(in == -1)
        in = commandTable[cur].pipeIn;
    if(out == -1)
        out= commandTable[cur].pipeOut;
    if(in!=-1)
        dup2(in,STDIN_FILENO);// close STDIN and make a copy of in
    if(out!=-1)
        dup2(out,STDOUT_FILENO);
}

void callChild()
{
    int status =0;
    for(int i=0;i<commandNum;i++)
    {
        trans(i);// parse
        openFilesAndPipes(i);//open files
        childPid = fork();//create
        if(childPid==0)
        {
            changeIO(i);// redirect and pipe
            closeFilesAndPipes(i);//close all
            int state = execvp(commandsPara[0],commandsPara);// detect excevp success or not
            if(state == -1)
                exit(EXIT_FAILURE);//exit with return 1 not continue to execute other code in main()
        }
        else
        {
            closeFilesAndPipes(i);//close files
            waitpid(childPid,&status,0);// waiting
            // if(status!=0)
                // printf("WRONG\n");
            // printf("child process finished\n");
            // closeFiles(i);
        }
        
    }
}

int main(int argc,char ** argv)
{
    signal(SIGINT,sigcat);
    while(1)
    {
        initCommand();
        int cnt = 0,flag = 0;
        char temp ;
        while(1)
        {
            temp = getchar();
            if(temp == '\n')
            {
                if(cnt == 0)
                    flag=1;
                break;
            }
            else if(temp == EOF)
            {
                if(cnt ==0 )
                    return 0;
                flag = 2;
                break;
            }
            commandInput[cnt++]= temp;
        }
        commandInput[cnt] = '\0';
        if(flag==1)
            continue;
        if(strcmp(commandInput,"exit")==0)
            break;
        praser();
        callChild();
        if (flag == 2)
            break;
    }
    return 0;
}