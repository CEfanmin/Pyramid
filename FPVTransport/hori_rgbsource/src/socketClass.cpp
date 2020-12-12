#include "../../hori_rgbsource/include/socketClass.h"
#define SOCKET_ERROR 0
mySocket::mySocket(){
    paramSetDone = false;
    socketType = 0;
    port = 10086;
    queuelen = 20;
}

mySocket::~mySocket(){
}

bool mySocket::serverInit(void){
    if(paramSetDone) return false;
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    on = 1;
	ret = setsockopt( server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind");
        return false;
    }

    if(listen(server_sockfd,queuelen) == -1)
    {
        perror("listen");
        return false;
    }

    sockaddr_in client_addr;
    length = sizeof(client_addr);

    conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
    if(conn<0)
    {
        perror("connect");
        return false;
    }
    paramSetDone = true;
    return true;
}

int mySocket::serverRecv(void *buff,int len){
    int lenth = recv(conn, buff, len,0);
    return lenth;
}

int mySocket::serverSend(void *buff,int len){
    int lenth = send(conn, buff, len, 0);
    return lenth;
}

bool mySocket::serverClose(void){
    close(conn);
    close(server_sockfd);
    return true;
}

int mySocket::serverAutoReconnect(int Port)
{
    if(paramSetDone) return -1;
    port = Port;
    if(!serverInit())return -1;
    socketType = 1;
    return 0;
}

bool mySocket::clientInit(char *ip){
    if(paramSetDone) return false;
    sock_cli = socket(AF_INET,SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);  ///服务器端口
    servaddr.sin_addr.s_addr = inet_addr(ip);  ///服务器ip
    while(connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0);
    paramSetDone = true;
    return true;
}

int mySocket::clientRecv(void *buff,int len){
    int lenth = recv(sock_cli, buff, len,0);
    return lenth;
}

int mySocket::clientSend(void *buff,int len){
    //std::cout << "before send" << std::endl;
    int lenth = send(sock_cli, buff, len,0);
    //std::cout << "sent" << lenth << std::endl;
    return lenth;
}

bool mySocket::clientClose(void){
    close(sock_cli);
    return true;
}

int mySocket::clientAutoReconnect(char* ip, int Port)
{
    if(paramSetDone) return -1;
    port = Port;
    if(!clientInit(ip))return -1;
    socketType = 2;
    return 0;
}


int mySocket::serverReceiveOnce(void *buff,int len){
    if(!serverInit())return -1;
    int length = serverRecv(buff,len);
    serverClose();
	return length;
}

int mySocket::clientSendOnce(char *ip,void *buff,int len){
    if(!clientInit(ip))return -1;
    int length = clientSend(buff,len);
    clientClose();
	return length;
}

int mySocket::Send(void* buff, int len,char *ip)
{
    if(socketType == 0){
        return -1;
    }
    else if(socketType == 1){
        int length = serverSend(buff,len);
        //std::cout << "len:" << length << std::endl;
        if(SOCKET_ERROR == length){
            serverClose();
            paramSetDone = false;
            serverInit();
            return -1;
        }
        return length;
    }
    else if(socketType == 2){
        int length = clientSend(buff,len);
        //std::cout << "len:" << length << std::endl;
        if(SOCKET_ERROR == length){
            clientClose();
            paramSetDone = false;
            clientInit(ip);
            return -1;
        }
        return length;
    }
}

int mySocket::Receive(void* buff, int len,char *ip)
{
    if(socketType == 0){
        return -1;
    }
    else if(socketType == 1){
        int length = serverRecv(buff,len);
        //std::cout << "len:" << length << std::endl;
        if(SOCKET_ERROR == length){
            serverClose();
            paramSetDone = false;
            serverInit();
            return -1;
        }
        return length;
    }
    else if(socketType == 2){
        int length = clientRecv(buff,len);
        //std::cout << "len:" << length << std::endl;
        if(SOCKET_ERROR == length){
            clientClose();
            paramSetDone = false;
            clientInit(ip);
            return -1;
        }
        return length;
    }
}
