#include "vision.h"

void _surfStyleMatching(const Mat& queryDescriptors,
		vector<vector<DMatch> > matchesKnn, vector<DMatch>& matches12);

const float MAX_DISTANCE_TO_MATCH = 40.0f;

// Setup the detector, extractor, and matcher
//ORB::CommonParams cp = ORB::CommonParams(1.2f, 5U, 10, 1, 2); 

const int DEFAULT_QUERY_FEATURES = 300;
const int DEFAULT_TRAINING_FEATURES = 300;
/*
Ptr<ORB> getQueryDetector() {
	Ptr<ORB> detector = ORB::create(DEFAULT_QUERY_FEATURES, 1.2f, 5U, 10, 1, 2);
	return detector;
}

Ptr<ORB> getTrainerDetector() {
	return getTrainerDetector(DEFAULT_TRAINING_FEATURES);
}
Ptr<ORB> getTrainerDetector(int detection_points) {
	Ptr<ORB> detector = ORB::create(DEFAULT_QUERY_FEATURES, 1.2f, 5U, 10, 1, 2);
	return detector;
}

Ptr<DescriptorMatcher> getMatcher() {
	Ptr<DescriptorMatcher> descriptorMatcher = DescriptorMatcher::create(
			"BruteForce-HammingLUT");
	//Ptr<DescriptorMatcher> descriptorMatcher = new lsh::LshMatcher();

	return descriptorMatcher;
	//return (Ptr<DescriptorMatcher>) descriptorMatcher;
}
*/
void surfStyleMatching(Ptr<DescriptorMatcher>& descriptorMatcher,
		const Mat& queryDescriptors, vector<DMatch>& matches12) {
	vector<vector<DMatch> > matchesKnn;

	descriptorMatcher->radiusMatch(queryDescriptors, matchesKnn,
			MAX_DISTANCE_TO_MATCH);

	_surfStyleMatching(queryDescriptors, matchesKnn, matches12);
}

void surfStyleMatching(Ptr<DescriptorMatcher>& descriptorMatcher,
		const Mat& queryDescriptors, const Mat& trainingDescriptors,
		vector<DMatch>& matches12) {
	vector<vector<DMatch> > matchesKnn;

	descriptorMatcher->radiusMatch(queryDescriptors, trainingDescriptors,
			matchesKnn, MAX_DISTANCE_TO_MATCH);

	_surfStyleMatching(queryDescriptors, matchesKnn, matches12);
}

void _surfStyleMatching(const Mat& queryDescriptors,
		vector<vector<DMatch> > matchesKnn, vector<DMatch>& matches12) {

	//objectMatches.clear();
	//objectMatches.resize(objectIds.size());
	//cout << "starting matcher" << matchesKnn.size() << endl;
	for (int descInd = 0; descInd < queryDescriptors.rows; descInd++) {
		const std::vector<DMatch> & matches = matchesKnn[descInd];
		//cout << "two: " << descInd << ":" << matches.size() << endl;

		// Check to make sure we have 2 matches.  I think this is always the case, but it doesn't hurt to be sure
		if (matchesKnn[descInd].size() > 1) {

			// Next throw out matches with a crappy score
			// Ignore... already handled by the radiusMatch
			//if (matchesKnn[descInd][0].distance < MAX_DISTANCE_TO_MATCH)
			//{
			float ratioThreshold = 0.75;

			// Check if both matches came from the same image.  If they both came from the same image, score them slightly less harshly
			if (matchesKnn[descInd][0].imgIdx
					== matchesKnn[descInd][1].imgIdx) {
				ratioThreshold = 0.85;
			}

			if ((matchesKnn[descInd][0].distance
					/ matchesKnn[descInd][1].distance) < ratioThreshold) {
				bool already_exists = false;
				// Quickly run through the matches we've already added and make sure it's not a duplicate...
				for (unsigned q = 0; q < matches12.size(); q++) {
					if (matchesKnn[descInd][0].queryIdx
							== matches12[q].queryIdx) {
						already_exists = true;
						break;
					} else if ((matchesKnn[descInd][0].trainIdx
							== matches12[q].trainIdx)
							&& (matchesKnn[descInd][0].imgIdx
									== matches12[q].imgIdx)) {
						already_exists = true;
						break;
					}
				}
				// Good match.
				if (already_exists == false)
					matches12.push_back(matchesKnn[descInd][0]);
			}

			//}
		} else if (matchesKnn[descInd].size() == 1) {
			// Only match?  Does this ever happen?
			matches12.push_back(matchesKnn[descInd][0]);
		}
		// In the ratio test, we will compare the quality of a match with the next match that is not from the same object:
		// we can accept several matches with similar scores as long as they are for the same object. Those should not be
		// part of the model anyway as they are not discriminative enough

		//for (unsigned int first_index = 0; first_index < matches.size(); ++first_index)
		//{

		//matches12.push_back(match);
		//}
	}

}

Mat trainImage(const Mat& img, OrbFeatureDetector& detector,
		Ptr<DescriptorMatcher>& descriptorMatcher) {

	//cout << "< Computing descriptors for keypoints from first image..." << endl;
	Mat descriptors;
	vector<KeyPoint> keypoints;
    detector.detect(img, keypoints);
    detector.compute(img, keypoints, descriptors);
	//cout << keypoints1.size() << " points" << endl << ">" << endl;

	//cout << ">" << endl;

	//assert(!img.empty());

	//cout << "  -- Found " << keypoints.size() << " keypoints" << endl;

	return descriptors;
}

RecognitionResult recognize(const Mat& queryImg, bool drawOnImage,
        Mat* outputImage, Ptr<FeatureDetector>& detector,
        Ptr<DescriptorMatcher>& descriptorMatcher,
        Ptr<DescriptorExtractor> descriptorExtractor,
        vector<string>& billMapping,
		bool debug_on, vector<int> debug_matches_array) {

    RecognitionResult result;
	result.haswinner = false;

	Mat queryDescriptors;
	vector<KeyPoint> queryKeypoints;
    detector->detect(queryImg, queryKeypoints);
    descriptorExtractor->compute(queryImg, queryKeypoints, queryDescriptors);

    if (queryKeypoints.size() <= 5) {
		// Cut it loose if there's less than 5 keypoints... nothing would ever match anyway and it could crash the matcher.
		if (drawOnImage) {
			drawKeypoints(queryImg, queryKeypoints, *outputImage,
					Scalar(0, 255, 0), DrawMatchesFlags::DEFAULT);
		}
		return result;
	}

	vector<DMatch> filteredMatches;

	surfStyleMatching(descriptorMatcher, queryDescriptors, filteredMatches);

	// Create and initialize the counts to 0
	vector<int> bill_match_counts(billMapping.size(), 0);
	for (unsigned i = 0; i < filteredMatches.size(); i++) {
		bill_match_counts[filteredMatches[i].imgIdx]++;
		//if (filteredMatches[i].imgIdx
	}

	float max_score = 0;	// represented as a percent (0 to 100)
	int highest_score_bill_index = -1;
	for (unsigned i = 0; i < billMapping.size(); i++) {
		//cout << billMapping[i] << ": " << bill_match_counts[i];

		// Adjust the score by subtracting all other matches.
		int adjustedcount = bill_match_counts[i];
		int maxother = 0;
		for (unsigned k = 0; k < billMapping.size(); k++) {
			if (k == i)
				continue; // Don't subtract from myself
			if (bill_match_counts[k] > maxother)
				maxother = bill_match_counts[k];
			//adjustedcount = adjustedcount - bill_match_counts[k];
		}
		adjustedcount = adjustedcount - maxother;
		if (adjustedcount < 0)
			adjustedcount = 0;
		//cout << " : " << adjustedcount;

		float score = 0;
        if (adjustedcount >= 4) {
			score = ((float) (adjustedcount - 5)) / 25 * 100;
			if (score > 100)
				score = 100;
		}

		if (score > max_score) {
			max_score = score;
			highest_score_bill_index = i;
		}

		//cout << endl;
	}

	//if (highest_score_bill_index != -1)
	//  //cout << "And the winner is!!!  " << billMapping[highest_score_bill_index] << endl;
	//else
	//  cout << "Everyone's a loser!  " << endl;

	if (highest_score_bill_index != -1) {
		result.haswinner = true;
		string billInfo = billMapping[highest_score_bill_index];
		cout << "billInfo = " << billInfo << endl;
		string winnerInfo;
		if (billInfo.substr(1, 2) == "cn") {
			winnerInfo = "元人民币";
			if (billInfo.substr(3, (billInfo.length() - 8)) == "1")
				result.winner = "一" + winnerInfo;
			else if (billInfo.substr(3, (billInfo.length() - 8)) == "5")
				result.winner = "五" + winnerInfo;
			else if (billInfo.substr(3, (billInfo.length() - 8)) == "10")
				result.winner = "十" + winnerInfo;
			else if (billInfo.substr(3, (billInfo.length() - 8)) == "20")
				result.winner = "二十" + winnerInfo;
			else if (billInfo.substr(3, (billInfo.length() - 8)) == "50")
				result.winner = "五十" + winnerInfo;
			else if (billInfo.substr(3, (billInfo.length() - 8)) == "100")
				result.winner = "一百" + winnerInfo;
		} else if (billInfo.substr(1, 3) == "us") {
			winnerInfo = "us dollar";
			result.winner = billInfo.substr(2, (billInfo.length() - 8))
					+ winnerInfo;
		} else if (billInfo.substr(1, 2) == "uk") {
			winnerInfo = "pound";
			result.winner = billInfo.substr(2, (billInfo.length() - 8))
					+ winnerInfo;
		} else if (billInfo.substr(1, 2) == "ru") {
			winnerInfo = "rouble";
			result.winner = billInfo.substr(2, (billInfo.length() - 8))
					+ winnerInfo;
		}
		//example of template image name “0cn100f.jpg”

		//result.winner = billInfo.substr(2,(billInfo.length()-2-6)) + winnerInfo;
		if (billInfo.substr((billInfo.length() - 5), 1) == "f")
			result.winner = result.winner + "正面";
		else
			result.winner = result.winner + "反面";
		result.confidence = max_score;
	}

	if (drawOnImage) {
		vector<KeyPoint> positiveMatches;
		for (unsigned i = 0; i < filteredMatches.size(); i++) {
			if (filteredMatches[i].imgIdx == highest_score_bill_index) {
				positiveMatches.push_back(
						queryKeypoints[filteredMatches[i].queryIdx]);
			}
		}

		Mat tmpImg;
		drawKeypoints(queryImg, queryKeypoints, tmpImg, Scalar(185, 0, 0),
				DrawMatchesFlags::DEFAULT);
		//drawKeypoints(  img2, keypoints2, tmpImg, CV_RGB(185, 0, 0), DrawMatchesFlags::DEFAULT );
		drawKeypoints(tmpImg, positiveMatches, *outputImage, Scalar(0, 0, 255),
				DrawMatchesFlags::DEFAULT);

		if (result.haswinner == true) {

			std::ostringstream out;
			out << result.winner << " (" << result.confidence << "%)";

			//speak(billMapping[highest_score_bill_index]);
			// we detected a bill, let the people know!
			//putText(*outputImage, out.str(), Point(15, 27), FONT_HERSHEY_DUPLEX,
			//		1.1, CV_RGB(0, 0, 0), 2);
		}
	}

	if (debug_on) {
		//debug_matches_array = int[billMapping.size()];
		for (unsigned i = 0; i < billMapping.size(); i++)
			debug_matches_array[i] = bill_match_counts[i];
	}

	return result;

	//Mat drawImg;

	//end=clock();

	//cout << "Matches: " << filteredMatches.size() << endl;
	//cout << "filtered in: " << diffclock(begin, end) << "ms"  << endl;
	//cout << "E2E time:" << diffclock(truebegin, end) << "ms" << endl << ">" << endl;

	//drawKeypoints(  img2, keypoints2, drawImg, CV_RGB(185, 0, 0), DrawMatchesFlags::DEFAULT );
	//Mat tmpImg;
	//drawKeypoints(  img2, keypoints2, tmpImg, CV_RGB(185, 0, 0), DrawMatchesFlags::DEFAULT );
	//drawKeypoints(  tmpImg, positiveMatches, drawImg, CV_RGB(0, 0, 255), DrawMatchesFlags::DEFAULT );

	//if ((highest_score_bill_index != -1) && (prev_match_index == highest_score_bill_index))
	//{

	//std::ostringstream out;
	//out << billMapping[highest_score_bill_index] << " (" << max_score << "%)";

	//speak(billMapping[highest_score_bill_index]);
	// we detected a bill, let the people know!
	//putText(drawImg, out.str(), Point(15, 27), FONT_HERSHEY_DUPLEX, 1.1, CV_RGB(0, 0, 0), 2);
	//}
	//imshow( winName, drawImg );

	//prev_match_index = highest_score_bill_index;

}
