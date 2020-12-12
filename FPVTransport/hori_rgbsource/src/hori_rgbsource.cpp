/*
 * Author: fanmin
 * Date: 2016/07/25
 * Function: 基于xtion相机的RGB视频数据通过socket传输
*/
#include "ros/ros.h"
#include "sensor_msgs/Image.h"
#include "opencv2/opencv.hpp"

#include <pthread.h>
#include "../../hori_rgbsource/include/socketClass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static std::vector<uchar> encoded_buffer;
static std::vector<uchar> copy_encoded_buffer;
bool send_finish = true;
mySocket csocket;

void socket_client(std::vector<uchar>cop_encoded_buffer)
{
	for(;;)
	{
		int retlen = csocket.Send(&(copy_encoded_buffer.at(0)),copy_encoded_buffer.size(),(char *)"127.0.0.1");
		usleep(30*1000);   //30毫秒 缓解发送太快，丢一帧中的数据本机
		if(retlen == -1 )
		{
			perror("error in send \n");
			continue;
		}
		 else
		{
			send_finish = true;
		}
	}

}

void *thread_function(void *arg)
{
	 socket_client(copy_encoded_buffer);
	 return NULL;
}

void RGBCallback(const sensor_msgs::Image::ConstPtr& msg)
{
	std::cout<<"msg->data.size() is:" <<msg->data.size() <<std::endl;
	cv::Mat RGBdata(480, 640, CV_8UC3);
	memcpy(RGBdata.data,&(msg->data[0]),msg->data.size());

	cv::imshow("RGBSOURCE",RGBdata);
	cv::waitKey(1);

	std::vector<int> encode_params;
	encode_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	encode_params.push_back(90);

	static bool thread_begin = false;   //线程锁标志
	cv::imencode(".jpeg", RGBdata, encoded_buffer, encode_params);  //encoded_buffer中是纯的一帧视频数据流
/*
	  //测试编码后的jpeg格式的视频流信息，显示出来
	  static bool start1 = false;
	  static std::vector<uchar>realjpegimage;
	  std::cout<<"encoded_buffer size is "<< encoded_buffer.size() <<std::endl;
		for (int i=0; i< encoded_buffer.size()-1; ++i)
		{
			if ( (encoded_buffer.at(i)== 0xFF) && (encoded_buffer.at(i+1)==0xD8) && (start1==false))
			{
				std::cout<< "start i = " << i <<std::endl;
				start1 = true;
			}
			if (start1 == true )
			{
				realjpegimage.push_back(encoded_buffer.at(i));
			}
			if ((encoded_buffer.at(i)== 0xFF) && (encoded_buffer.at(i+1)==0xD9))
			{
				realjpegimage.push_back(0xD9);
				std::cout<< "end i+1 = " << i+1<<std::endl;
				start1 = false;
				std::cout<<"write data finish..."<<std::endl;
				break;
			}
		}
		if ((start1== false) && (realjpegimage.size() > 0))
		{
			std::cout<<"real data size is :"<< realjpegimage.size() << std::endl;
			//利用opencv:imdecode函数解码成Mat数据并显示
			cv::Mat matrixJpeg = cv::imdecode(cv::Mat(realjpegimage), CV_LOAD_IMAGE_COLOR);
			if ((matrixJpeg.cols>0) && (matrixJpeg.rows>0))
			{
				std::cout<<"imshow image..."<<std::endl;
				cv::imshow("MatImageofsend",matrixJpeg);
				cv::waitKey(1);
			}
			realjpegimage.clear();
		}
		//测试结束
*/
	 if (send_finish)
	  {
		  copy_encoded_buffer.assign(encoded_buffer.begin(), encoded_buffer.end());
		  send_finish= false;
		  std::cout<< "copyencoded buffer before socket is: " << copy_encoded_buffer.size()<<std::endl;
	  }

	  printf("start socket trans\n");   //开启线程
	  if (thread_begin == false)
	  {
		  	pthread_t a_thread;
		    int res = pthread_create(&a_thread, NULL, thread_function, NULL);
		    if (res != 0)
		    {
		        perror("Thread creation failed!");
		        exit(EXIT_FAILURE);
		    }
		  thread_begin = true;
	  }
	  printf("socket trans end!\n");

}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "hori_rgbsource_node");
	ros::NodeHandle n;
	csocket.serverAutoReconnect();

	ros::Subscriber sub = n.subscribe("/camera/rgb/image_color", 100, RGBCallback);
	ros::spin();
	return 0;
}
