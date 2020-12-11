#include <ros/ros.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>

#include "xtion_image/object.h"  //own msg

using namespace std;

int main(int argc, char* argv[])
{
	/*
	ros::init(argc, argv, "xtion_imagenode");
	ros::NodeHandle n;

	ros::Publisher chatter_pub = n.advertise<xtion_image::object>("/object_result", 1);
	ros::Rate loop_rate(1);

	xtion_image::object objects;
	while(ros::ok())
	{
		objects.name = "rainbow";
		objects.description = "a beautiful rainbow in the blue sky";
		objects.location[0] = 123;
		objects.location[1] = 456;
		objects.location[2] = 789;
		chatter_pub.publish(objects);

		loop_rate.sleep();
	}*/

    //检测关键点，提取特征向量
    cv::initModule_nonfree();//初始化
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create("SIFT");//create sift feature dector
    cv::Ptr<cv::DescriptorExtractor> descriptor_extractor = cv::DescriptorExtractor::create("SIFT");//create feature vector extractor
    cv::Ptr<cv::DescriptorMatcher> descriptor_matcher = cv::DescriptorMatcher::create("BruteForce");//create feature matcher
    if (detector.empty() || descriptor_extractor.empty())
    {
        return - 1;
    }

    //读取图片
    cv::Mat img1 = cv::imread("/home/fanmin/catkin_ws/src/xtion_image/dataset/test3.png"); //img1 is test
    cv::Mat img2 = cv::imread("/home/fanmin/catkin_ws/src/xtion_image/dataset/template.png");  //img2 is template(train)

    //检测keypoints
    std::vector<cv::KeyPoint> keypoint1, keypoint2;
    detector->detect(img1, keypoint1);
    detector->detect(img2, keypoint2);

    //根据keypoints计算128维度描述符
    cv::Mat descriptor1, descriptor2;
    descriptor_extractor->compute(img1, keypoint1, descriptor1);
    descriptor_extractor->compute(img2, keypoint2, descriptor2);

	//绘制keypoints
	cv::Mat img_keypoints1, img_keypoints2;
	drawKeypoints(img1, keypoint1, img_keypoints1);
	drawKeypoints(img2, keypoint2, img_keypoints2);
	//imshow("keypoints1", img_keypoints1);
	//imshow("keypoints2", img_keypoints2);


	//根据sift描述符匹配，并初步筛选
	std::vector<vector<cv::DMatch> > knn_match;
	descriptor_matcher->knnMatch(descriptor1, descriptor2, knn_match, 2);//返回最优匹配和次优匹配

	//正确匹配保证最优匹配和次优匹配之间具有较大差距，如果两者之间差距较小，则认为是错误匹配
	const float minRatio = 1.f / 1.5f;
	//const float minRatio = 0.5;
	std::vector<cv::DMatch> matches;
	for (int i = 0; i < knn_match.size(); i++){
		const cv::DMatch& bestMatch = knn_match[i][0];
		const cv::DMatch& betterMatch = knn_match[i][1];

		float r = bestMatch.distance / betterMatch.distance;
		if (r < minRatio)
			matches.push_back(bestMatch);
	}

	//绘制匹配结果
	cv::Mat img_knnMatches;
	drawMatches(img1, keypoint1, img2, keypoint2, matches, img_knnMatches);
	//imshow("KNN matches", img_knnMatches);

	//RANSAC计算单应变换矩阵排除错误匹配点对
	std::vector<cv::KeyPoint> alignedKps1, alignedKps2;
	for (int i = 0; i < matches.size(); i++){
		alignedKps1.push_back(keypoint1[matches[i].queryIdx]);
		alignedKps2.push_back(keypoint2[matches[i].trainIdx]);
	}
    std::vector<cv::Point2f> ps1, ps2;
	for (int i = 0; i < alignedKps1.size(); i++)
		ps1.push_back(alignedKps1[i].pt);

	for (int i = 0; i < alignedKps2.size(); i++)
		ps2.push_back(alignedKps2[i].pt);

	if (matches.size() < 4){
		std::cout<< "Matches number is not enough"<< std::endl;
		return -1;
	}

	cv::Mat status = cv::Mat::zeros(matches.size(), 1, CV_8UC1);
	cv::Mat H = findHomography(ps2, ps1, CV_FM_RANSAC, 3, status);

	std::vector<cv::DMatch> RANSAC_matches;
	uchar *status_p;
	std::vector<cv::DMatch>::const_iterator it_match = matches.begin();
	for (int i = 0; i < matches.size(); i++){
		status_p = status.ptr<uchar>(i);
		if (*status_p)
			RANSAC_matches.push_back(it_match[i]);
	}

	//绘制RANSAC筛选后的匹配结果
	cv::Mat img_RANSAC_matches;

	drawMatches(img1, keypoint1, img2, keypoint2, RANSAC_matches, img_RANSAC_matches);
	cv::imshow("RANSAC_matches", img_RANSAC_matches);

	//检测目标物体在场景图像中的位置
	cv::Mat img_detect = img1.clone();;
	std::vector<cv::Point2f> model_corners(4);//存储模板图像的四个角点
	std::vector<cv::Point2f> scene_corners(4);

	//确定模板在场景中的位置
	model_corners[0] = cvPoint(0, 0);//左上角为原点(0,0)；x轴指向→；y轴指向↓
	model_corners[1] = cvPoint(img2.cols, 0);
	model_corners[2] = cvPoint(img2.cols, img2.rows);
	model_corners[3] = cvPoint(0, img2.rows);

	cv::perspectiveTransform(model_corners, scene_corners, H);

	cv::line(img_detect, scene_corners[0], scene_corners[1], cv::Scalar(0, 255, 0), 4);
	cv::line(img_detect, scene_corners[1], scene_corners[2], cv::Scalar(0, 255, 0), 4);
    cv::line(img_detect, scene_corners[2], scene_corners[3], cv::Scalar(0, 255, 0), 4);
    cv::line(img_detect, scene_corners[3], scene_corners[0], cv::Scalar(0, 255, 0), 4);

    //求取中心点
    std::cout<< "scene_corners0 is:" << scene_corners[0] <<std::endl;
    std::cout<< "scene_corners2 is:" << scene_corners[2] <<std::endl;
	CvPoint box_center;
	box_center.x = (scene_corners[0].x + scene_corners[2].x)/2;
	box_center.y = (scene_corners[0].y + scene_corners[2].y)/2;

    std::cout<< "box_center is:" << box_center.x <<std::endl;
    std::cout<< "box_center is:" << box_center.y <<std::endl;
    cv::circle(img_detect, box_center,3, cv::Scalar(0, 0, 255), -1, 8, 0 );
    cv::imshow("Object detection", img_detect);
    cvWaitKey();

    return 0;
}




