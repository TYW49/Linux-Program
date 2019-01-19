//
// Created by codesky on 1/1/18.
//

#ifndef HELLOWORLD_COMMUNICATION_H
#define HELLOWORLD_COMMUNICATION_H

#endif //HELLOWORLD_COMMUNICATION_H

typedef struct msgbuf{
    long int mtype;
    char mtext[1024];
}MSG;


//消息队列操作

//创建消息队列,如果存在返回存在队列的id
int createMessageQueue(char* pathname,int id);

//根据id,删除消息队列
int removeMessageQueue(int msqId);

//发送消息到消息队列
int sendMessage(int msqid, char *text, long int type);



//管道操作

//读管道
int readPipeByLine(char *pathname,char *buf);

//写管道
int writePipe(char *pathname,char *buf);
