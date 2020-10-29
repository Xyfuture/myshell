#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

char words[] = "hello,world! \n";
char *commands[]={"grep","l",NULL};
char *commands2[]={"./output",NULL};
int main()
{
    int fds[2];
    pipe(fds);
    // write(fds[1],words,sizeof(words));
    pid_t pid1,pid2;
    pid1 = fork();
    if (pid1 == 0)
    {
        dup2(fds[1],STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        execvp(commands2[0],commands2);
    }
    else
    {
        int status;
        wait(&status); 
        pid2 = fork();
        if(pid2 == 0)
        {
            
            dup2(fds[0],STDIN_FILENO);
            close(fds[1]);
            close(fds[0]);
            execvp(commands[0],commands);
        }
        else
        {
            int status;
            wait(&status); 
            printf("finished");
        }
    }

    return 0;
}


