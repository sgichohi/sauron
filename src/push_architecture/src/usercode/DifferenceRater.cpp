#include "DifferenceRater.h"
#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>

#include <stdio.h>
#include <string.h>

#include <climits>
#include <iostream>

using namespace cv;
using namespace std;

namespace UserDefined {
  
  /*******************************/
  /* USER FACTORY IMPLEMENTATION */
  /*******************************/

  Sendable *UserFactory::getNewSendable() {
    return new SendableMat();
  }

  Transformer *UserFactory::getNewTransformer() {
    return new DifferenceRater();
  }

  /***********************************/
  /* DIFFERENCE RATER IMPLEMENTATION */
  /***********************************/

  DifferenceRater::DifferenceRater() {
    cur = NULL;
    full = false;
    num_features = 100;
    scale = 0.5;
    detector = ORB(num_features);
  }

  // Destructor
  DifferenceRater::~DifferenceRater() {
    if (cur != NULL) delete cur;
  }
    
  static bool compareMatches(DMatch m1, DMatch m2) {
     return m1.distance < m2.distance;
  }

  // Initialize the iterator
  void DifferenceRater::begin(Mat pic, long timestamp) {
    
    if (NULL == cur) {

      Mat small_pic, gray_frame;
      resize(pic, small_pic, Size(), scale, scale, INTER_NEAREST);
      cvtColor(small_pic, gray_frame, CV_BGR2GRAY);
      equalizeHist(gray_frame, gray_frame);

      detector.detect(gray_frame, cur_keypoints);
      detector.compute(gray_frame, cur_keypoints, cur_descriptors);

      SendableMat *sendable = new SendableMat();
      sendable->initialize(pic, timestamp, LONG_MAX);
      cur = sendable;
      full = true;

    } else {

      vector<KeyPoint> prev_keypoints = cur_keypoints;
      Mat prev_descriptors = cur_descriptors;
      
      Mat small_pic, gray_frame;
      resize(pic, small_pic, Size(), scale, scale, INTER_NEAREST);
      cvtColor(small_pic, gray_frame, CV_BGR2GRAY);
      detector.detect(gray_frame, cur_keypoints);
      detector.compute(gray_frame, cur_keypoints, cur_descriptors);

      BruteForceMatcher<L2<float> > matcher;
      vector<DMatch> matches;
      matcher.match(prev_descriptors, cur_descriptors, matches);

      // Compute the "difference between frames
      int num_matches_to_consider =
        min(prev_keypoints.size(), cur_keypoints.size()) / 2;
      
      std::sort(matches.begin(), matches.end(), compareMatches);
      float total_squared_distance = 0;
      for (int i = 0; i < num_matches_to_consider; ++i) {
        DMatch m = matches.at(i);
        total_squared_distance += m.distance*m.distance;
      }
      float normalized_distance = sqrt(total_squared_distance) / sqrt(num_matches_to_consider);
      
      vector<KeyPoint> scale_adjusted_keypoints;
      for (vector<KeyPoint>::iterator it = cur_keypoints.begin(); it != cur_keypoints.end(); it++) {
        Point2f pt(it->pt.x/scale, it->pt.y/scale);
        scale_adjusted_keypoints.push_back(KeyPoint(pt, it->size));
      }

      // Draw the keypoints, just for fun
      drawKeypoints(pic, scale_adjusted_keypoints, pic, Scalar(0, 0, 255));

      long score = long(normalized_distance);

      delete cur;
      SendableMat *sendable = new SendableMat();
      sendable->initialize(pic, timestamp, score);
      cur = sendable;
      full = true;
    }
  }
    
  // Check whether the iterator is finished
  bool DifferenceRater::finished() {
    return !full;
  }
  
  // Move the iterator to the next value
  void DifferenceRater::next() {
    full = false;
  }

  Sendable *DifferenceRater::current() {
    return cur;
  }
}
