/*
 * xtion_image.h
 *
 *  Created on: Dec 8, 2016
 *      Author: fanmin
 */

#ifndef  XTION_IMAGE_H
#define XTION_IMAGE_H

#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <std_msgs/String.h>

class rgb_image
{
public:

	cv::Mat RGBFrame;
	cv::Mat copyRGBFrame;
	CvPoint box_center;
	rgb_image(const std::string& ImageTopic);
	~rgb_image(){};
    void captureRGBImage(const sensor_msgs::ImageConstPtr& msg);
    void ObjectDetect(cv::Mat copyRGBFrame);

private:
    ros::NodeHandle nh_;
    image_transport::ImageTransport rgbImage_;
	image_transport::Subscriber rgbImageSub_;
	ros::Publisher position_pub_;
};

#endif // XTION_IMAGE_H
