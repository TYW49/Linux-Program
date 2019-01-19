//
// Created by codesky on 1/1/18.
//

#include "communication.h"
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>

int createMessageQueue(char *pathname, int id) {
    int msqid;
    key_t key;
    if ((key = ftok(pathname, id)) == -1) {
        perror("ftok");
        return -1;
    }
    if ((msqid = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        if (errno != EEXIST) {
            perror("msgget");
            return -1;
        }
        if ((msqid = msgget(key, 0)) == -1) {
            perror("msgget");
            return -1;
        }
    }
    return msqid;
}


int removeMessageQueue(int msqId) {
    if (msgctl(msqId, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return -1;
    }
    return 0;
}


int sendMessage(int msqid, char *text, long int type) {
    MSG msg;
    memset(&msg, 0x00, sizeof(MSG));
    msg.mtype = type;
    memcpy(msg.mtext, text, strlen(text));
    if (msgsnd(msqid, &msg, strlen(msg.mtext), IPC_NOWAIT) == -1) {
        perror("msgsnd");
        return -1;
    }
    return 0;
}

int receiveMessage(int msqid, MSG *msg, long int type) {
    memset(msg, 0x00, sizeof(MSG));
    if (msgrcv(msqid, msg, strlen(msg->mtext), type, 0) == -1) {
        perror("msgrcv");
        return -1;
    }
    return 0;
}


int readPipeByLine(char *pathname,char *buf){


    return 0;
}
