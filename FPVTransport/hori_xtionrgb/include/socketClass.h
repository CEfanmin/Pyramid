#pragma once
#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

class mySocket{
public:
    mySocket();
    ~mySocket();
private:
    //common
    bool paramSetDone;
    int socketType;//0 NULL , 1 Server , 2 Client
    int port;
    int queuelen;

    //server
    int server_sockfd;
    struct sockaddr_in server_sockaddr;
	int on;
	int ret;
    struct sockaddr_in client_addr;
    socklen_t length;
    int conn;

    //client
    int sock_cli;
    struct sockaddr_in servaddr;


public:
    void changePort(int Port){
        port = Port;
    }
    void changeQueuelen(int len){
        queuelen = len;
    }

    bool serverInit(void);
    int serverRecv(void *buff,int len);
    int serverSend(void *buff,int len);
    bool serverClose(void);
    int serverAutoReconnect(int Port = 10086);

    bool clientInit(char *ip);
    int clientRecv(void *buff,int len);
    int clientSend(void *buff,int len);
    bool clientClose(void);
    int clientAutoReconnect(char *ip,int Port = 10086);

    int serverReceiveOnce(void *buff,int len);
    int clientSendOnce(char *ip,void *buff,int len);

    int Send(void *buff,int len,char *ip = NULL);
    int Receive(void *buff,int len,char *ip = NULL);
};
