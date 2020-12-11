/*
 * main.cpp
 *
 *  Created on: Dec 8, 2016
 *      Author: fanmin
 */

#include <iostream>
#include <ros/ros.h>
#include <pthread.h>
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/UInt16.h>
#include "xtion_image.h"

using namespace std;
ros::Publisher *p =NULL;
double min_range_=0;
double max_range_=1;

void captureDepthImage(const sensor_msgs::ImageConstPtr& image)
{
	cv_bridge::CvImagePtr bridge;
	try
	{
		bridge = cv_bridge::toCvCopy(image, "32FC1");
	}
	catch (cv_bridge::Exception& e)
	{
		ROS_ERROR("Failed to transform depth image.");
		return;
	}
	cv::Mat img(bridge->image.rows, bridge->image.cols, CV_8UC1);
	for(int i = 0; i < bridge->image.rows; i++)
	{
		float* Di = bridge->image.ptr<float>(i);
		char* Ii = img.ptr<char>(i);
		for(int j = 0; j < bridge->image.cols; j++)
		{
			Ii[j] = (char) (255*((Di[j]-min_range_)/(max_range_-min_range_)));
		}
	}
	cv::imshow("DepthImage", img);
	cv::waitKey(1);
	float pos = bridge->image.at<float>(150,150);
	std::cout<< "position is: "<< pos <<std::endl;
	std_msgs::UInt16 msg;
	msg.data = pos;
	p->publish(msg);
}

void *thread_function(void *arg)
{
	while(ros::ok())
	{
		std::cout<< "get position..." <<std::endl;
		sleep(1);
	}
    return NULL;
}

int main(int argc, char ** argv)
{
	ros::init(argc, argv, "xtion_image_node");
	ros::NodeHandle n;
	ros::Subscriber depth_sub = n.subscribe("/camera/depth/image_raw", 1, captureDepthImage);
	ros::Publisher chatter_pub = n.advertise<std_msgs::UInt16>("/position_result", 1);
	p = &chatter_pub;

	std::string topicRGB = "/camera/rgb/image_color";
	rgb_image RGBimage(topicRGB);

//	pthread_t a_thread;
//	int res = pthread_create(&a_thread, NULL, thread_function, NULL);
//	if (res!= 0)
//	{
//		perror("Thread creation failed!");
//		exit(EXIT_FAILURE);
//	}

	ros::spin();
	return 0;
}
