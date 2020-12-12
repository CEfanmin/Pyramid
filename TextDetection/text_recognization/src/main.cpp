#include <ros/ros.h>
#include "std_msgs/String.h"
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

#include "text_main.h"
#include "image_process.h"

using namespace std;
using namespace cv;

//extern int my_main(cv::Mat &img, vector<string> &recog_result, cv::Mat &text_result);   "chi_sim" "eng"
	
int main(int argc, char** argv)
{
	ros::init(argc, argv, "text_recognization");
	ros::NodeHandle n;
	ros::Publisher chatter_pub = n.advertise<std_msgs::String>("/text_result", 1);

	imageCapture myCapture("/camera/rgb/image_rect_color");

	srand((int)time(0));
	Mat image;
	vector<string> recog_result;
	cv::Mat text_result;
	namedWindow("image",1);

	Mat frame;
    ros::Rate loop_rate(1);

	while(ros::ok() && frame.empty())
	{
		frame = myCapture.RGBImage_;
		loop_rate.sleep();
		ros::spinOnce();
	}
	while(ros::ok())
	{
		char filename[200];
		while(myCapture.busy_)
		{
			//TODO: WAITING
		}

		frame = myCapture.RGBImage_; // get a new frame from camera

		imshow("image", frame);
		my_main(frame, recog_result, text_result,"eng");

		cout<<recog_result[0]<<endl;	//recog_result为输出的结果
		//if ()                         //26个字母的ASCII码的范围作为输入
		//recog_result[0].data()

		std_msgs::String msg;
		msg.data = recog_result[0];
		chatter_pub.publish(msg);

		cv::imshow("result",text_result);
		waitKey(10);
		recog_result.clear();
		loop_rate.sleep();
		ros::spinOnce();
	}

	return 0;
}
