/*
 * image_converter.h
 *
 *  Created on: Jan 21, 2016
 *      Author: fanmin
 */

#ifndef IMAGE_PROCESS_INCLUDE_IMAGE_CONVERTER_H_
#define IMAGE_PROCESS_INCLUDE_IMAGE_CONVERTER_H_

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <std_msgs/String.h>

#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

#include <iostream>

#include "stdafx.h"
#include "speaker.h"
#include "vision.h"
#include "fingertipdetect.h"
#include "imgtrainer.h"

class currency_recognization
{
public:
 currency_recognization(const std::string& rgbImageTopicName,
	const std::string& resultTopicName)
	: rgbImage_(nh_),
	  wintime(0)
	 {

	 	  rgbImageSub_ = rgbImage_.subscribe(rgbImageTopicName, 1,
	 			  &currency_recognization::captureRGBImage, this);  //回调函数为captureRGBImage

	 	  recognizationResultPub_  = nh_.advertise<std_msgs::String>(resultTopicName, 1);
	 	  init();
	 }

 int init(void)
 {
	    detector            = FeatureDetector::create("ORB");
	    descriptorExtractor = DescriptorExtractor::create("ORB");
	    descriptorMatcher   = DescriptorMatcher::create("BruteForce-HammingLUT");

	    if (detector.empty() || descriptorMatcher.empty()) {
	        cout
	                << "Can not create detector or descriptor exstractor or descriptor matcher of given types"
	                << endl;
	    }

	    cout << " Loading the template images..." << endl;
	    string a="us";

	    int success=loadRecognitionSet(a,descriptorMatcher, billMapping);

		cout << "billMapping.size() = " << billMapping.size() << endl;
		if (success == 0)
		{
			cout << "Failed training images, exiting....." << endl;
			return -1;
		}
		return 0;
 }
 void recognization(void)
 {
		double t_all = (double) getTickCount();
		float scale_img = (float) (600.f / qureyFrame.rows);
		float scale_font = (float) (2 - scale_img) / 1.4f;

		cv::resize(qureyFrame, mgray_small, Size(320, 240));
		Mat drawImg;
		vector<int> debug_matches(billMapping.size(), 0);
		RecognitionResult result = recognize(mgray_small, true, &drawImg,
             detector, descriptorMatcher, descriptorExtractor, billMapping, true, debug_matches);

     if (result.haswinner == true) {

         if(wintime>1){
              wintime = 0;
             //AutoSpeak(result.winner);
             cout << "result = " << result.winner << endl;

             //TODO:publish result
             std_msgs::String recognizationResult;
             recognizationResult.data = result.winner;
             recognizationResultPub_.publish(recognizationResult);

             cout << "time eplase = "
                     << ((double) getTickCount() - t_all) * 1000./ getTickFrequency() << endl;
         }
         wintime++;
		}
		t_all = ((double) getTickCount() - t_all) * 1000 / getTickFrequency();
		char buff[100];
		sprintf(buff, "%2.1f Fps. @ 320x240", (float) (1000 / t_all));
		string fps_info = buff;
		putText(drawImg, fps_info, Point(10, drawImg.rows - 10),
				FONT_HERSHEY_DUPLEX, scale_font, Scalar(255, 0, 0));
		imshow("coin", drawImg);
		if (waitKey(10) == 27) {
			cout << "Enter ESC to stop it" << endl;
			return;
		}
 }

	void captureRGBImage(const sensor_msgs::ImageConstPtr& msg)
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

		//ROS_INFO("update image");
		qureyFrame = cv_ptr->image;
		recognization();
	}

private:
	ros::NodeHandle nh_;
	image_transport::ImageTransport rgbImage_;
	image_transport::Subscriber rgbImageSub_;
	ros::Publisher recognizationResultPub_;

    int wintime;//=0;
	cv::Mat qureyFrame;
	cv::Mat mgray_small;
    Ptr<FeatureDetector>     detector;
    Ptr<DescriptorExtractor> descriptorExtractor;
    Ptr<DescriptorMatcher>   descriptorMatcher;
	vector<string> billMapping;
};

#endif /* IMAGE_PROCESS_INCLUDE_IMAGE_CONVERTER_H_ */
