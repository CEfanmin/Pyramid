#!/usr/bin/python

import argparse as ap
import cv2
import imutils 
import numpy as np
import os
import time 
from sklearn.externals import joblib
from scipy.cluster.vq import *

import roslib
import rospy
import sys
from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError

# Load the classifier, class names, scaler, number of clusters and vocabulary 
clf, classes_names, stdSlr, k, voc = joblib.load("bof.pkl")

# Create feature extraction and keypoint detector objects
fea_det = cv2.FeatureDetector_create("SIFT")
des_ext = cv2.DescriptorExtractor_create("SIFT")

class image_classification:
    def __init__(self):
        self.image_pub = rospy.Publisher("/classification_result", String, queue_size=1)
        self.bridge = CvBridge()
        self.image_sub = rospy.Subscriber("/camera/rgb/image_color", Image, self.RGBsource)

    def RGBsource(self,data):
        cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
        (rows, cols, channels) = cv_image.shape
        if cols > 0 and rows > 0 :
            kpts = fea_det.detect(cv_image)
            kpts, des = des_ext.compute(cv_image, kpts)
            descriptors = des
            test_features = np.zeros((1, k), "float32")
            words, distance = vq(descriptors, voc)
            
            for w in words:
                test_features[0][w] += 1

            # Perform Tf-Idf vectorization
            nbr_occurences = np.sum((test_features > 0) * 1, axis=0)
            idf = np.array(np.log((1.0*1+1) / (1.0*nbr_occurences + 1)), 'float32')

            # Scale the features
            test_features = stdSlr.transform(test_features)

            # Perform the predictions
            predictions = [classes_names[i] for i in clf.predict(test_features)]
            
            # Visualize the results, if "visualize" flag set to true by the user
            print "prediction is:", predictions[0]
            self.image_pub.publish(predictions[0])               
            cv2.namedWindow("Image", cv2.WINDOW_NORMAL)
            pt = (0, 3 * cv_image.shape[0] // 4)
            cv2.putText(cv_image, predictions[0], pt ,cv2.FONT_HERSHEY_SCRIPT_COMPLEX, 2, [0, 255, 0], 2)
            cv2.imshow("Image", cv_image)
            cv2.waitKey(1)
              
def main(args):
    ic = image_classification()
    rospy.init_node('image_classifiction', anonymous=True)
    try:
        rospy.spin()
    except KeyboardInterrupt:
        print("Shutting down")
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main(sys.argv) 