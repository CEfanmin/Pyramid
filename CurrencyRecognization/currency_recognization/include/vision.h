#ifndef VISION_H_
#define VISION_H_
#include "stdafx.h"

Ptr<ORB> getQueryDetector();

Ptr<ORB> getTrainerDetector();

Ptr<ORB> getTrainerDetector(int detection_points);

Ptr<DescriptorMatcher> getMatcher();

struct RecognitionResult {
	bool haswinner;
	string winner;
	int confidence;
};

void surfStyleMatching(Ptr<DescriptorMatcher>& descriptorMatcher,
		const Mat& queryDescriptors, const Mat& trainingDescriptors,
		vector<DMatch>& matches12);

void surfStyleMatching(Ptr<DescriptorMatcher>& descriptorMatcher,
		const Mat& queryDescriptors, vector<DMatch>& matches12);

Mat trainImage(const Mat& img1, Ptr<ORB>& detector,
		Ptr<DescriptorMatcher>& descriptorMatcher);

RecognitionResult recognize(const Mat& queryImg, bool drawOnImage,
        Mat* outputImage, Ptr<FeatureDetector>& detector,
        Ptr<DescriptorMatcher>& descriptorMatcher, Ptr<DescriptorExtractor> descriptorExtractor, vector<string>& billMapping,
		bool debug_on, vector<int> debug_matches_array);

#endif
