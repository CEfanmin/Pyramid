/*
 * image_process.h
 *
 *  Created on: Apr 13, 2016
 *      Author: assassin
 */

#ifndef TEXT_RECOGNIZATION_INCLUDE_IMAGE_PROCESS_H_
#define TEXT_RECOGNIZATION_INCLUDE_IMAGE_PROCESS_H_
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <std_msgs/String.h>

#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

class imageCapture
{
public:
 imageCapture(const std::string& rgbImageTopicName)
	: rgbImage_(nh_),
	  busy_(false)
 {

	  rgbImageSub_ = rgbImage_.subscribe(rgbImageTopicName, 1,
		&imageCapture::captureRGBImage, this);
 }

void captureRGBImage(const sensor_msgs::ImageConstPtr& msg)
{
	busy_ = true;

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

	//ROS_INFO("update image");
	RGBImage_ = cv_ptr->image;;
	busy_ = false;
}

private:
	ros::NodeHandle nh_;
	image_transport::ImageTransport rgbImage_;
	image_transport::Subscriber rgbImageSub_;
public:
	cv::Mat RGBImage_;
	bool busy_;
};



#endif /* TEXT_RECOGNIZATION_INCLUDE_IMAGE_PROCESS_H_ */
