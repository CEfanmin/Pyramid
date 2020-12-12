#ifndef IMGTRAINER_H_
#define IMGTRAINER_H_
#include "stdafx.h"

int loadRecognitionSet(string name, Ptr<DescriptorMatcher>& descriptorMatcher, vector<string>& billMapping);
//void initCurrencyDetector();
double fps(clock_t clock1, clock_t clock2);

void imshowMany(const std::string& _winName, const vector<Mat>& _imgs);

void stratUp(Ptr<DescriptorMatcher> &descriptorMatcher,
		vector<string> &billMapping, Ptr<ORB> &detector);

#endif
