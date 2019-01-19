#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <stdlib.h>
#include <pwd.h>
#include "communication.h"

#define MAXLINE 1024 //the max length of commmand line data
#define MAXARG 20     //max num of arg


char namedPipe[] = "/home/codesky/pipe/p1";
char msqPath[] = "/etc/profile";
char msg_type = 1;
char *buffer;

void type_prompt();

int read_command(char **command, char **parameters);


int main(int argc, char *argv[], char *env[]) {
    printf("front program start.\n");

    char *command = NULL;  //命令
    char **parameters;     //参数数组
    parameters = malloc(sizeof(char *) * (MAXARG + 2));
    buffer = malloc(sizeof(char) * MAXLINE);
    char CommandAndArgs[MAXLINE];  //转换后的命令+参数字符串

    int msqid = createMessageQueue(msqPath, 1);
    int pid = fork();
    if (pid == 0) {
        if (execl("/home/codesky/back", "back", NULL) == -1) {
            perror("execl");
            exit(0);
        }
    } else {
        //接受用户输入命令
        while (1) {
            //清空字符串
            memset(CommandAndArgs,0x00, sizeof(CommandAndArgs));
            memset(buffer,0x00, sizeof(CommandAndArgs));
            //输出终端提示符  [codesky@ubuntu ~] #
            type_prompt();
            //读取命令
            int count=read_command(&command, parameters);

            //处理windows命令到Linux命令的转换(注意：无效命令处理)
            /* windows命令  -->  Linux命令
             *
             * dir    -- ls -l
             * rename -- mv
             * move   -- mv
             * cd     -- pwd
             * del    -- rm  -rf filename
             * exit   -- exit
             *
             */
            if(strcmp(command,"dir")==0){
                command="ls -l";
                strcpy(CommandAndArgs,command);
            }else if (strcmp(command,"rename")==0){
                command="mv";
                strcpy(CommandAndArgs,command);
                for(int i=1;i<count-1;i++){
                    strcat(CommandAndArgs," ");
                    strcat(CommandAndArgs,parameters[i]);
                }
            }else if(strcmp(command,"move")==0){
                command="mv";
                strcpy(CommandAndArgs,command);
                for(int i=1;i<count-1;i++){
                    strcat(CommandAndArgs," ");
                    strcat(CommandAndArgs,parameters[i]);
                }
            }else if(strcmp(command,"cd")==0){
                //内建命令  区分对待
                if(count>2){
                    chdir(parameters[1]);
                    strcat(CommandAndArgs,"cd ");
                    strcat(CommandAndArgs,parameters[1]);
                }else{
                    command="pwd";
                    strcpy(CommandAndArgs,command);
                }
            }else if(strcmp(command,"del")==0){
                command="rm";
                strcpy(CommandAndArgs,command);
                for(int i=1;i<count-1;i++){
                    strcat(CommandAndArgs," ");
                    strcat(CommandAndArgs,parameters[i]);
                }
            }else if (strcmp(command,"exit")==0){
                strcpy(CommandAndArgs,"exit");
            } else{
                printf("No command '%s' found\n",command);
                continue;
            }
            //发送正确命令到消息队列
            sendMessage(msqid, CommandAndArgs, msg_type);
            if (strcmp(command, "exit") == 0) {
                break;
            }
            usleep(200);
            //从管道读取命令返回结果
            int pipe = open(namedPipe, O_RDONLY);
            char buf;
            while (read(pipe, &buf, 1) > 0) {
                write(STDOUT_FILENO, &buf, 1);
            }
            close(pipe);
        }
    }
    return 0;
}






int read_command(char **command, char **parameters) {

    char *Res_fgets = fgets(buffer, MAXLINE, stdin);
    if (Res_fgets == NULL) {
        printf("\n");
        exit(0);
    }

    if (buffer[0] == '\0')
        return -1;
    char *pStart, *pEnd;
    int count = 0;
    int isFinished = 0;
    pStart = pEnd = buffer;
    while (isFinished == 0) {
        while ((*pEnd == ' ' && *pStart == ' ') || (*pEnd == '\t' && *pStart == '\t')) {
            pStart++;
            pEnd++;
        }

        if (*pEnd == '\0' || *pEnd == '\n') {
            if (count == 0)
                return -1;
            break;
        }

        while (*pEnd != ' ' && *pEnd != '\0' && *pEnd != '\n')
            pEnd++;


        if (count == 0) {
            char *p = pEnd;
            *command = pStart;
            while (p != pStart && *p != '/')
                p--;
            if (*p == '/')
                p++;
            parameters[0] = p;
            count += 2;
//            printf("\ncommand:%s\n", *command);
        } else if (count <= MAXARG) {
            parameters[count - 1] = pStart;
            count++;
        } else {
            break;
        }

        if (*pEnd == '\0' || *pEnd == '\n') {
            *pEnd = '\0';
            isFinished = 1;
        } else {
            *pEnd = '\0';
            pEnd++;
            pStart = pEnd;
        }
    }

    parameters[count - 1] = NULL;
    /*
    printf("input analysis:\n");
    printf("pathname:[%s]\ncommand:[%s]\nparameters:\n", *command, parameters[0]);
    int i;
    for (i = 0; i < count - 1; i++)
        printf("[%s]\n", parameters[i]);
    */
    return count;
}


//输出终端提示符
void type_prompt() {
    /*
   系统文件/etc/passwd包含一个用户帐号数据库。它由行组成，每行对应一个用户，包括：
   用户名、加密口令、用户标识符（UID）、组标识符（GID）、全名、主目录和默认shell。
   编程接口的数据结构：
   strcut passwd {
       char *pw_name;
       char *pw_passwd;
       uid_t pw_uid;
       gid_t pw_gid;
       char *pw_gecos;
       char *pw_dir;
       char *pw_shell;
   }
   */
    struct passwd *pw;
    pw = getpwuid(getuid());  //获取当前登陆用户信息

    char hostname[256];
    gethostname(hostname, sizeof(hostname)); //获取主机名

    //获取当前目录信息
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(0);
    }
    printf("%s@%s:%s $", pw->pw_name, hostname, cwd);  //输出终端提示符
}