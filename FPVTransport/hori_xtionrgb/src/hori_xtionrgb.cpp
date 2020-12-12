/*
 * Author: fanmin
 * Date: 2016/07/26
 * Function: 获取hori URL的jpeg视频数据
*/
#include "ros/ros.h"

#include <image_transport/image_transport.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../../hori_xtionrgb/include/socketClass.h"

#define DEFAULT_PORT 8002
#define MAXLINE 160*1024
using namespace std;

mySocket csocket;
int main(int argc, char** argv)
{
	ros::init(argc, argv, "rgbimage_publisher");
	ros::NodeHandle nh;
	image_transport::ImageTransport it(nh);
	image_transport::Publisher pub = it.advertise("/hori_rgb_image", 1);

	csocket.clientAutoReconnect("127.0.0.1");
    char*   pbuff = new char[MAXLINE];
    int     number = 0;
    int     ret = 0;
    char*   videodata =new char[MAXLINE];
    static std::vector<uchar>realjpegimage;
    static bool start1 = false;
    printf("\n======waiting for client's request======\n");

    for(;;)
    {
    	//循环接收客户端传过来的数据
    	do
    	{
    		//number = recv(connect_fd, pbuff, MAXLINE, 0);
    		number = csocket.Receive(pbuff,MAXLINE,"127.0.0.1");
    		memcpy(videodata,pbuff,number);
    		std::vector<uchar>jpegimage(videodata, videodata+ number);

    		//display_data(jpegimage);
		   for (int i=0; i< jpegimage.size(); ++i)
		   {
			if ( (jpegimage.at(i)== 0xFF) && (jpegimage.at((i+1)>= jpegimage.size() ? jpegimage.size()-1: i+1)==0xD8) && (start1==false))
			{
				start1 = true;
			}

			if (start1 == true )
			{
				realjpegimage.push_back(jpegimage.at(i));
			}

			if ((jpegimage.at(i)== 0xFF) && (jpegimage.at( (i+1) >= jpegimage.size() ? jpegimage.size()-1: i+1)==0xD9))
			{
				realjpegimage.push_back(0xD9);
				start1 = false;
				std::cout<<"write data finish..."<<std::endl;
				break;
			}
		  }
		if ((start1== false) && (realjpegimage.size() > 0))
		{
			std::cout<<"real data size is :"<< realjpegimage.size() <<std::endl;
			//利用opencv:imdecode函数解码成Mat数据并显示
			cv::Mat matrixJpeg = cv::imdecode(cv::Mat(realjpegimage), CV_LOAD_IMAGE_COLOR);
			if ((matrixJpeg.cols>0) && (matrixJpeg.rows>0))
			{
				std::cout<<"imshow image...\n"<<std::endl;
				//cv::namedWindow("MatImage", CV_WINDOW_NORMAL);  //自动调节窗口大小
				cv::imshow("RGBImage",matrixJpeg);
				cv::waitKey(1);
				sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", matrixJpeg).toImageMsg();
				ros::Rate loop_rate(5);
				pub.publish(msg);
			}
			realjpegimage.clear();
		}

    	}while(number>=0);
        delete [] pbuff;
    }
	return 0;
}

