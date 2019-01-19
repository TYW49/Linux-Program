#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include "communication.h"

char info[] = "success\n";
char namedPipe[] = "/home/codesky/pipe/p1";
char msqPath[] = "/etc/profile";
long int msg_type = 1;


int main() {
    printf("back program start.\n");
    //初始化
    int msqid = createMessageQueue(msqPath, 1);
    MSG msg;
    //消息队列读取命令
    while (1) {
        receiveMessage(msqid, &msg, msg_type);
        //popen执行命令，从匿名管道读取命令返回结果（注意：mv rm命令不返回信息，所以向前台输出success）
        FILE *fd;
        char command[1024];
        int isWrite = 0;  //命令是否返回结果标志
        //处理命令
        memcpy(command, msg.mtext, sizeof(command));
        if (strcmp(command, "exit") == 0) {
            break;
        }

        //打开管道  写入命令返回结果
        int pipe = open(namedPipe, O_WRONLY);
        if(command[0]=='c'&&command[1]=='d'){
            chdir(command+3);
        }else {
            if ((fd = popen(command, "r")) == NULL) {
                perror("popen");
            }
            char buf[1024];
            //命令返回信息写入管道
            while (fgets(buf, sizeof(buf), fd) != NULL) {
                isWrite = 1;
                if (write(pipe, buf, strlen(buf)) == -1) {
                    perror("pipe write");
                }
            }
            pclose(fd); //关闭匿名管道
        }
        if (!isWrite) {
            write(pipe, info, strlen(info));
        }
        close(pipe);  //关闭命名管道
    }
    return 0;
}