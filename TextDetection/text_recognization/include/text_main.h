#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv/cv.h>

#include <iostream>
#include <vector>
#include <string>
using namespace std;

vector<string> my_recognition(cv::Mat gray,cv::Mat &image,std::string chooseL);
int my_main(cv::Mat &img, vector<string> &recog_result, cv::Mat &text_result, std::string chooseL);
