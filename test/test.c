#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
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
char *commandsPara[COMMAND_NUM]; // for execvp 每个命令现场生成
char reInFile[COMMAND_BUF];
char reOutFile[COMMAND_BUF];
int allPipes[PIPE_COUNT][2];

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
            commandTable[commandNum].pipeOut = pipeNum;
            commandTable[++commandNum].pipeIn = pipeNum;
            commandTable[commandNum].startPosi = cursor+1;
            pipeNum++;
        }
        else
            continue;
    }
    commandTable[commandNum].endPosi = cursor-1;
    commandNum++;
}

void initCommand()
{
    commandInfo temp = {-1,-1,-1,-1,-1,-1};
    for(int i=0;i<COMMAND_COUNT;i++)
        commandTable[i] = temp;
    // for(int i=0;i<PIPE_COUNT;i++)
    //     allPipes[i][0]=-1,allPipes[i][1]=-1;
    // for(int j=0;j<COMMAND_NUM;j++)
    //     commandsPara[j] = NULL;
    memset(commandInput,'\0',sizeof(commandInput));
    memset(commands,0,sizeof(commands));
    memset(allPipes,-1,sizeof(allPipes));
    memset(reInFile,0,sizeof(reInFile));
    memset(reOutFile,0,sizeof(reOutFile));
    memset(commandsPara,0,sizeof(commandsPara));

}

int redirect(int s,int e,int type)
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
    return s;
}

void trans(int cur)
{
    int s = commandTable[cur].startPosi;
    int e = commandTable[cur].endPosi;
    int cnt = 0,j=0,t=0;
    while(commandInput[s]==' ')
        s++;
    while(commandInput[e]==' ')
        e--;
    printf("cur: %d\n",cur);
    printf("s:%d e:%d\n",s,e);
    for(int i=s;i<=e;i++)
    {
        if(commandInput[i]==' ')
        {
            while(commandInput[i]==' '&&i<=e)
                i++;
            i--;
            if(j==0)
                continue;
            commands[cnt][j] = '\0';
            commandsPara[t++] = commands[cnt];
            j=0;
            cnt++;
            //printf("here: %s\n",commands[cnt-1]);
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

void openFilesAndPipes(int cur)
{
    // may need this
    if(commandTable[cur].pipeIn!=-1)
        if (allPipes[commandTable[cur].pipeIn][0]==-1)
            pipe(allPipes[commandTable[cur].pipeIn]);
    if(commandTable[cur].pipeOut!=-1)
        if (allPipes[commandTable[cur].pipeOut][1]==-1)
            pipe(allPipes[commandTable[cur].pipeOut]);            
    if(commandTable[cur].reIn!=-1)
        commandTable[cur].reIn = open(reInFile,O_RDONLY);
    if(commandTable[cur].reOut!=-1)
        commandTable[cur].reOut = open(reOutFile,O_WRONLY);
}

void closeFiles(int cur)//close redirect files in father process
{
    if(commandTable[cur].reIn!=-1)
        close(commandTable[cur].reIn);
    if(commandTable[cur].reOut!=-1)
        close(commandTable[cur].reOut);
    // pipes close in the end of all execs
}

void closePipes()
{
    for(int i=0;allPipes[i][0]!=-1;i++)
        close(allPipes[i][0]),close(allPipes[i][1]);
}

void changeIO(int cur)// in child process
{
    int in=commandTable[cur].reIn;
    int out=commandTable[cur].reOut;
    if(in ==-1)
        in = commandTable[cur].pipeIn;
    if(out==-1)
        out= commandTable[cur].pipeOut;
    if(in!=-1)
    {
        close(0);
        dup(in);
        close(in);
    }
    if(out!=-1)
    {
        close(1);
        dup(out);
        close(out);
    }
}

int main(int argc,char ** argv)
{
    initCommand();
    scanf("%[^\n]",commandInput);
    praser();
    for(int i=0;i<commandNum;i++)
    {
        //printf("start:%d   end:%d\n",commandTable[i].startPosi,commandTable[i].endPosi);
        //printf("pipeS:%d   pipeE:%d\n",commandTable[i].pipeIn,commandTable[i].pipeOut);
        trans(i);
        for(int j=0;commandsPara[j]!=NULL;j++)
           printf("%s\n",commandsPara[j]);
        printf("in:%d   out:%d\n",commandTable[i].reIn,commandTable[i].reOut);
    }
    return 0;
}