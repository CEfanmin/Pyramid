/*
 * Author: fanmin
 * Date: 2016/07/25
 * Function: 基于xtion相机的Depth视频数据通过socket传输
*/
#include "ros/ros.h"
#include "sensor_msgs/Image.h"
#include "opencv2/opencv.hpp"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>

#include <cv_bridge/cv_bridge.h>

static std::vector<uchar> encoded_buffer;
static std::vector<uchar> copy_encoded_buffer;
bool send_finish = true;

char host_name[20];
int  port = 8008;
void socket_client(std::vector<uchar>cop_encoded_buffer)
{
	  //const char * host_name = "127.0.0.1";
	  const char * host_name = "192.168.1.110";
	  int socket_descriptor;
	  struct sockaddr_in pin;

	  bzero(&pin,sizeof(pin));
	  pin.sin_family = AF_INET;
	  inet_pton(AF_INET,host_name,&pin.sin_addr);
	  pin.sin_port = htons(port);

	  if((socket_descriptor =  socket(AF_INET,SOCK_STREAM,0)) == -1)
	  {
			  perror("error opening socket \n");
			  exit(1);
	  }
	  if(connect(socket_descriptor,(struct sockaddr * )&pin,sizeof(pin)) == -1)  //阻塞的连接
	  {
			  perror("error connecting to socket \n");
			  exit(1);
	  }

	  for(;;)
	  {
		  int retlen =  write(socket_descriptor, &(copy_encoded_buffer.at(0)), copy_encoded_buffer.size());

		  usleep(30*1000);   //30毫秒   TODO: 缓解发送太快，丢一帧中的数据 本机

		  printf("retlen is: %d\n", retlen);
		  if(retlen == -1 )
		  {

			  perror("error in send \n");
			  exit(1);
		  }
		  else
		  {
			  send_finish = true;
		  }
	  }

	  close(socket_descriptor);

}

void *thread_function(void *arg)
{
	 socket_client(copy_encoded_buffer);
	 return NULL;
}


void DepthCallback(const sensor_msgs::Image::ConstPtr& msg)
{
	cv::Mat img;
    if (msg->encoding.find("F") != std::string::npos)
    {
      cv::Mat float_image_bridge = cv_bridge::toCvCopy(msg, msg->encoding)->image;   //将sensor_msgs转码后复制给float_image_bridge
      cv::Mat_<float> float_image = float_image_bridge;

      //归一化显示图像
      double max_val;
      cv::minMaxIdx(float_image, 0, &max_val);
      if (max_val > 0)
      {
        float_image *= (255/max_val);
      }

      img = float_image;
    }

    else
    {
      std::cout<<"Convert to OpenCV native BGR color..." <<std::endl;
      img = cv_bridge::toCvCopy(msg, "bgr8")->image;
    }

    int input_width = img.cols;
    int input_height = img.rows;

    cv::Mat img_resized;
	cv::Size new_size(input_width, input_height);
	cv::resize(img, img_resized, new_size);
    cv::Mat Depthdata;
    Depthdata = img_resized;

	std::vector<int> encode_params;
	encode_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	encode_params.push_back(90);  //编码的品质０~100

	static bool thread_begin = false;   //线程锁标志
	cv::imencode(".jpeg", Depthdata, encoded_buffer, encode_params);  //encoded_buffer中是纯的一帧视频数据流
/*
	  //测试编码后的jpeg格式的视频流信息，显示出来
	  static bool start1 = false;
	  static std::vector<uchar>realjpegimage;
	  std::cout<<"encoded_buffer size is "<< encoded_buffer.size() <<std::endl;
		for (int i=0; i< encoded_buffer.size(); ++i)
		{
			if ( (encoded_buffer.at(i)== 0xFF) && (encoded_buffer.at((i+1)>=encoded_buffer.size() ? encoded_buffer.size():i+1)==0xD8) && (start1==false))
			{
				std::cout<< "start i = " << i <<std::endl;
				start1 = true;
			}
			if (start1 == true )
			{
				realjpegimage.push_back(encoded_buffer.at(i));
			}
			if ((encoded_buffer.at(i)== 0xFF) && (encoded_buffer.at((i+1)>=encoded_buffer.size() ? encoded_buffer.size():i+1)==0xD9))
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

	//cv::imshow("xtion_Depthimage",Depthdata);
	//cv::waitKey(1);
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "hori_Depthsource_node");

  ros::NodeHandle n;

  ros::Subscriber sub = n.subscribe("/camera/depth/image", 100, DepthCallback);
  
  ros::spin();
  return 0;
}
