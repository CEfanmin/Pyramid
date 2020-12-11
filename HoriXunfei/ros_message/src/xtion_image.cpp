/*
 * xtion_image.cpp
 *
 *  Created on: Dec 8, 2016
 *      Author: fanmin
 */

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <std_msgs/String.h>

#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "xtion_image.h"

rgb_image::rgb_image(const std::string& rgbImageTopic)
	:rgbImage_(nh_)
{
	rgbImageSub_ = rgbImage_.subscribe(rgbImageTopic, 1, &rgb_image::captureRGBImage, this);  //call back captureRGBImage
    //position_pub_= nh_.advertise<std_msgs::String>("/box_center", 1);
}

void rgb_image::captureRGBImage(const sensor_msgs::ImageConstPtr& msg)
{
	cv_bridge::CvImagePtr cv_ptr;
	try
	{
	  cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
	}
	catch (cv_bridge::Exception& e)
	{
	  ROS_ERROR("cv_bridge exception: %s", e.what());
	  return;
	}

	RGBFrame = cv_ptr->image;
	if (RGBFrame.cols >0 && RGBFrame.rows>0)
	{
		rgb_image::ObjectDetect(RGBFrame);
	}
	else
	{
		std::cout<<"RGBFrame size is error!" <<std::endl;
	}

}

void rgb_image::ObjectDetect(cv::Mat copyRGBFrame)
{
	rectangle(copyRGBFrame, cvPoint(100, 100), cvPoint(200, 200), cvScalar(0, 0, 255), 3, 4, 0 ); //TODO object detection bbox
	cv::imshow("RGBimage", copyRGBFrame);
	cvWaitKey(1);
	box_center.x = 150;  //TODO auto get center position
	box_center.y = 150;
}



