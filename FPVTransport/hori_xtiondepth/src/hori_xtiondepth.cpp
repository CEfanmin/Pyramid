/*
 * Author: fanmin
 * Date: 2016/07/10
 * Function: 获取hori URL的jpeg视频数据
*/
#include "ros/ros.h"
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>

#include<sys/socket.h>
#include<netinet/in.h>

#define DEFAULT_PORT 8001
#define MAXLINE 160*1024

using namespace std;
void display_data(std::vector<uchar>jpegimage)
{
	  //printf("start display...");
	  static std::vector<uchar>realjpegimage;
	  static bool start1 = false;

	  for (int i=0; i< jpegimage.size(); ++i)
	  {
		//printf("%02x ",jpegimage.at(i));
		if ( (jpegimage.at(i)== 0xFF) && (jpegimage.at((i+1)>= jpegimage.size() ? jpegimage.size()-1: i+1)==0xD8) && (start1==false))
		{
			//std::cout<< "start i = " << i <<std::endl;
			start1 = true;
		}

		if (start1 == true )
		{
			realjpegimage.push_back(jpegimage.at(i));
		}

		if ((jpegimage.at(i)== 0xFF) && (jpegimage.at( (i+1) >= jpegimage.size() ? jpegimage.size()-1: i+1)==0xD9))
		{
			realjpegimage.push_back(0xD9);
			//std::cout<< "end i+1 = " << i+1<<std::endl;   i=nmemb-1
			start1 = false;
			std::cout<<"write data finish..."<<std::endl;
			break;
		}
	 }

	if ((start1== false) && (realjpegimage.size() > 0))
	{
		std::cout<<"real data size is :"<< realjpegimage.size() <<std::endl;

		//利用opencv:imdecode函数解码成Mat数据并显示
		cv::Mat matrixJpeg = cv::imdecode(cv::Mat(realjpegimage), CV_LOAD_IMAGE_UNCHANGED);

		if ((matrixJpeg.cols>0) && (matrixJpeg.rows>0))
		{
			std::cout<<"imshow image..\n."<<std::endl;

			//cv::namedWindow("MatImage", CV_WINDOW_NORMAL);  //自动调节窗口大小
			cv::imshow("MatImage",matrixJpeg);
			cv::waitKey(1);
		}
		realjpegimage.clear();
	}
}


void robotcamera_socket()
{
    char*   pbuff = new char[MAXLINE];
    int     number = 0;
    int     ret = 0;
    char*   videodata =new char[MAXLINE];

    //初始化地址和端口
    struct  sockaddr_in  servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址
    servaddr.sin_port = htons(DEFAULT_PORT);//设置的端口为DEFAULT_PORT

    //初始化Socket
    int  socket_fd, connect_fd;
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
    	printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
    	exit(1);
    }

    //端口复用设置
    int bReuseaddr=1;
    ret = setsockopt (socket_fd,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bReuseaddr));

    //将本地地址绑定到所创建的套接字上
    if( bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
    	printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
    	exit(1);
    }

    //开始监听是否有客户端连接
    if( listen(socket_fd, 10) == -1)
    {
    	printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
    	exit(1);
    }
    printf("\n======waiting for client's request======\n");
    for(;;)
    {
    	//阻塞直到有客户端连接，不然多浪费CPU资源。
    	if( (connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1)
    	{
    		printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
    		continue;
    	}
    	else
    	{
    		printf("\nConnected!\n");
    	}
    	printf("start do while()\n");

    	//循环接收客户端传过来的数据
    	do
    	{
    		//printf("number is: %d\n",number);
    		number = recv(connect_fd, pbuff, MAXLINE, 0);

    		memcpy(videodata,pbuff,number);

    		std::vector<uchar>jpegimage(videodata, videodata+ number);

    		display_data(jpegimage);

    	}while(number>=0);

        close(connect_fd);
        close(socket_fd);
        delete [] pbuff;
    }
}

int main()
{
	robotcamera_socket();
	return 0;
}















