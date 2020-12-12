#include "imgtrainer.h"

int loadRecognitionSet(string name, Ptr<DescriptorMatcher>& descriptorMatcher,
		vector<string>& billMapping) {

	ifstream fReadFName("/home/fanmin/catkin_ws/src/currency_recognization/imagelist.txt");
	string oneline;
	vector<Mat> desc_set;
	Mat desc;
	vector<KeyPoint> kpts;
    Ptr<FeatureDetector>     detector            = FeatureDetector::create("ORB");
    Ptr<DescriptorExtractor> descriptorExtractor = DescriptorExtractor::create("ORB");

	while (getline(fReadFName, oneline)) {
		billMapping.push_back(oneline);
        string imgpath = "/home/fanmin/catkin_ws/src/currency_recognization/template/" + oneline;
		//imgpath.erase(imgpath.end() - 1);
		//cout<<imgpath<<endl;
		Mat img = imread(imgpath);
		if (img.empty()) {
			cout << "Can not read tempalte images.." << endl;
			return 0;
		}
        detector->detect(img, kpts);
        descriptorExtractor->compute(img, kpts, desc);
		desc_set.push_back(desc);
	}

	descriptorMatcher->add(desc_set);
    descriptorMatcher->train();

 //   FileStorage fileobj = FileStorage("/home/hd/programfiles/myqt/currency_recog/trained_descriptors.xml", FileStorage::WRITE);
 //   descriptorMatcher->write(fileobj);
 //   fileobj.release();

	return 1;
}

double fps(clock_t clock1, clock_t clock2) {
	double diffticks = clock2 - clock1;
	double fps = CLOCKS_PER_SEC / diffticks;
	return fps;
}

void imshowMany(const std::string& _winName, const vector<Mat>& _imgs) {
	int nImg = (int) _imgs.size();
	Mat dispImg;
	int size;
	int x, y;

	// w - Maximum number of images in a row
	// h - Maximum number of images in a column
	int w, h;
	// scale - How much we have to resize the image
	float scale;
	int max;

	if (nImg <= 0) {
		printf("Number of arguments too small....\n");
		return;
	} else if (nImg > 12) {
		printf("Number of arguments too large....\n");
		return;
	}

	else if (nImg == 1) {
		w = h = 1;
		size = 300;
	} else if (nImg == 2) {
		w = 2;
		h = 1;
		size = 300;
	} else if (nImg == 3 || nImg == 4) {
		w = 2;
		h = 2;
		size = 300;
	} else if (nImg == 5 || nImg == 6) {
		w = 3;
		h = 2;
		size = 200;
	} else if (nImg == 7 || nImg == 8) {
		w = 4;
		h = 2;
		size = 200;
	} else {
		w = 4;
		h = 3;
		size = 150;
	}

	dispImg.create(Size(100 + size * w, 60 + size * h), CV_8UC3);
	//Mat dispImg(100 + size*w, 60 + size*h, CV_8UC3);

	for (int i = 0, m = 20, n = 20; i < nImg; i++, m += (20 + size)) {
		x = _imgs[i].cols;
		y = _imgs[i].rows;

		max = (x > y) ? x : y;
		scale = (float) ((float) max / size);

		if (i % w == 0 && m != 20) {
			m = 20;
			n += 20 + size;
		}

		Mat imgROI = dispImg(Rect(m, n, (int) (x / scale), (int) (y / scale)));
		resize(_imgs[i], imgROI, Size((int) (x / scale), (int) (y / scale)));
	}

	namedWindow(_winName);
	imshow(_winName, dispImg);
}
/*
void stratUp(Ptr<DescriptorMatcher> descriptorMatcher,
		vector<string> billMapping, Ptr<ORB> detector) {
	cout << ">" << endl;
	if (detector.empty() || descriptorMatcher.empty()) {
		cout
				<< "Can not create detector or descriptor exstractor or descriptor matcher of given types"
				<< endl;
		return;
	}
	cout << "< Reading the images..." << endl;

	int success = loadRecognitionSet("us", descriptorMatcher, billMapping);
	cout << "billMapping.size() = " << billMapping.size() << endl;
	if (success == 0) {
		cout << "Failed training images, exiting....." << endl;
		return;
	}
}
*/
