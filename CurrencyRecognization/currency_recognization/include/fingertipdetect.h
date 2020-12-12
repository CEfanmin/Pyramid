#ifndef FINGERTIPDETECT_H_
#define FINGERTIPDETECT_H_
#include "stdafx.h"

//extract skin part from the image
void SkinExtract(const Mat &frame, Mat &skinArea);

//find the max skin contoure as the palm
int FindMaxAreaContoure(const vector<vector<Point> > &contours);

//get the center of the palm and the finger tip
pair<Point, Point> FindFingerTip(const vector<Point> maxcontour);

//get the rectangle ROI according to the info of finger tip
Rect GetRectROI(const Point &fingerTip, const Size frameSize, int discount);

int GetCircleROI(const Point &fingerTip, const Size frameSize);

//draw the boundary box
void DrawRectangle(const Rect & rectROI, Mat &frame);

int FingerDetector(Mat frame);

#endif
