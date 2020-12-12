#include "currency_recognization.h"

//TODO: Need to change two paths fReadFName and imgpath in imgtrainer.cpp

int main(int argc, char** argv)
{
	ros::init(argc, argv, "currency_recognization_node");
	std::string subtopic = "/camera/rgb/image_rect_color";
	std::string pubtopic = "/prmi/currency_recogniztion";
	currency_recognization recognization(subtopic, pubtopic);	//TODO: subscribe xtion topic from openni

	ros::spin();

	return 0;
}
