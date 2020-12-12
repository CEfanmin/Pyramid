#include "fingertipdetect.h"

void SkinExtract(const Mat &frame, Mat &skinArea) {
	Mat YCbCr;
	vector<Mat> planes;
	cvtColor(frame, YCbCr, 37);
	split(YCbCr, planes);
	//test
	//imshow("B_chanel", planes[1]);
	//imshow("R_chanel", planes[2]);
	//imshow("R_chanel", planes[3]);
	MatIterator_<uchar> it_Cb = planes[1].begin<uchar>(), it_Cb_end =
			planes[1].end<uchar>();
	MatIterator_<uchar> it_Cr = planes[2].begin<uchar>();
	MatIterator_<uchar> it_skin = skinArea.begin<uchar>();

	//100<=Cb<=127, 138<=Cr<=170
	for (; it_Cb != it_Cb_end; ++it_Cr, ++it_Cb, ++it_skin) {
		if (138 <= *it_Cr && *it_Cr <= 170 && 100 <= *it_Cb && *it_Cb <= 127)
			*it_skin = 255;
		else
			*it_skin = 0;
	}
	dilate(skinArea, skinArea, Mat(5, 5, CV_8UC1), Point(-1, -1));
	erode(skinArea, skinArea, Mat(5, 5, CV_8UC1), Point(-1, -1));
}

int FindMaxAreaContoure(const vector<vector<Point> > &contours) {
	int index = 0;
	double area = 0, maxArea(0);
	for (int i = 0; i < contours.size(); i++) {
		area = contourArea(Mat(contours[i]));
		if (area > maxArea) {
			maxArea = area;
			index = i;
		}
	}
	return index;
}

pair<Point, Point> FindFingerTip(const vector<Point> maxcontour) {
	Moments moment = moments(maxcontour, true);
	Point center(moment.m10 / moment.m00, moment.m01 / moment.m00);
	Point tmp;
	int max(0), notice(0), sumdist(0);
	for (int i = 0; i < maxcontour.size() && center.y > maxcontour[i].y; ++i)// 锟斤拷证指锟斤拷锟斤拷锟斤拷锟斤拷锟絚enter.y > couPoint[notice].y
			{
		tmp = maxcontour[i];
		int dist = (tmp.x - center.x) * (tmp.x - center.x)
				+ (tmp.y - center.y) * (tmp.y - center.y);
		if (dist > max) {
			max = dist;
			notice = i;
		}
		sumdist += dist;
	}
	if (max > 4 * sumdist / maxcontour.size()) {
		return make_pair(center, maxcontour[notice]);
	} else {
		return make_pair(center, Point(-1, -1));
	}
}
// gte rectangle Region of interest
Rect GetRectROI(const Point &fingerTip, const Size frameSize, int discount) {
	discount = 8;
	Rect rectROI;
	if (fingerTip.x <= frameSize.width / 2) {
		rectROI.x = fingerTip.x * (10 - discount) / 10;
		rectROI.y = fingerTip.y * (10 - discount) / 10;
		rectROI.width = fingerTip.x * discount * 2 / 10;
		rectROI.height = fingerTip.y * discount / 10;
	} else {
		rectROI.x = fingerTip.x
				- (frameSize.width - fingerTip.x) * discount / 10;
		rectROI.y = fingerTip.y * (10 - discount) / 10;
		rectROI.width = (frameSize.width - fingerTip.x) * 2 * discount / 10;
		rectROI.height = fingerTip.y * discount / 10;
	}
	return rectROI;
}
// gte circle Region of interest
int GetCircleROI(const Point &fingerTip, const Size frameSize) {
	int radius(0);
	radius =
			MIN(MIN(fingerTip.x, fingerTip.y),
					MIN(frameSize.width - fingerTip.x, frameSize.height - fingerTip.y))
					* 2 / 3;
	return radius;
}

// draw rectangle
void DrawRectangle(const Rect & rectROI, Mat &frame) {
	Point p1(rectROI.x, rectROI.y), p2(rectROI.x, rectROI.y + rectROI.height),
			p3(rectROI.x + rectROI.width, rectROI.y + rectROI.height), p4(
					rectROI.x + rectROI.width, rectROI.y);
	line(frame, p1, p2, Scalar(0, 255, 255), 2);
	line(frame, p2, p3, Scalar(0, 0, 255), 2);
	line(frame, p3, p4, Scalar(0, 255, 0), 2);
	line(frame, p4, p1, Scalar(0, 255, 0), 2);
}

int FingerDetector(Mat frame) {
	Mat skinArea;
	skinArea.create(frame.rows, frame.cols, CV_8UC1);
	SkinExtract(frame, skinArea);
	Mat show_img(frame.rows, frame.cols, CV_8UC3);
	frame.copyTo(show_img, skinArea);
	show_img = frame;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//
	findContours(skinArea, contours, hierarchy, 2, 2);
//	findContours(skinArea, contours, hierarchy, CV_RETR_CCOMP,
//			CV_CHAIN_APPROX_SIMPLE);
	//
	if (!contours.empty()) {
		int index = FindMaxAreaContoure(contours);
		//drawContours(frame, contours, index, Scalar(0, 0, 255), 2, CV_FILLED, hierarchy );
		pair<Point, Point> centerAndTip = FindFingerTip(contours[index]);
		if (centerAndTip.second == Point(-1, -1)) {
			cout << "can't detect the finger tip" << endl;
			return -1;
		}
		/*
		 // draw
		 int radius = GetCircleROI(centerAndTip.second,frame.size());
		 circle(show_img, centerAndTip.second, radius ,Scalar(255, 255, 0), 2);
		 Rect rectROI = GetRectROI(centerAndTip.second, frame.size(), 8);
		 DrawRectangle(rectROI, frame);
		 Mat frameROI(frame, rectROI);
		 //vector<Rect> P = FaceDetecor(frameROI);
		 circle(show_img, centerAndTip.second, 6 ,Scalar(0, 0, 255), CV_FILLED);
		 circle(show_img, centerAndTip.first, 6 ,Scalar(0, 255, 0), CV_FILLED);
		 line(show_img, centerAndTip.first, centerAndTip.second, Scalar(255, 0, 0), 2);
		 imshow("show_img", show_img);
		 */
	}
	return 1;
}

