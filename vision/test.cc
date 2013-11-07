#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>

using namespace cv;

bool compareMatches(DMatch m1, DMatch m2) { return m1.distance < m2.distance; }

int main(int nargs, char** args)
{
  int significance_threshold = -1;
  if (nargs > 1) {
    std::string s = string(args[1]);
    std::istringstream(s) >> significance_threshold;
  }

  VideoCapture cap(0);
  if(!cap.isOpened()) return -1;

  int num_features = 1000;
  ORB detector(num_features);

  Mat holding_frame, current_frame, last_frame;
  vector<KeyPoint> current_keypoints, last_keypoints;
  Mat current_descriptors, last_descriptors;
  vector<DMatch> matches;

  BruteForceMatcher<L2<float> > matcher;

  cap >> holding_frame;
  cvtColor(holding_frame, current_frame, CV_BGR2GRAY);
  detector.detect(current_frame, current_keypoints);
  detector.compute(current_frame, current_keypoints, current_descriptors);

  namedWindow("features", 1);
  Mat features, display, blank;

  for(;;)
    {
      last_frame = current_frame;
      cap >> holding_frame;
      
      cvtColor(holding_frame, current_frame, CV_BGR2GRAY);
      
      last_keypoints = current_keypoints;
      detector.detect(current_frame, current_keypoints);

      last_descriptors = current_descriptors;
      detector.compute(current_frame, current_keypoints, current_descriptors);

      matcher.match(last_descriptors, current_descriptors, matches);

      // Compute the "difference" between frames
      int num_matches_to_consider =
        min(last_keypoints.size(), current_keypoints.size()) / 2;
      
      std::sort(matches.begin(), matches.end(), compareMatches);
      float total_squared_distance = 0;
      for (int i = 0; i < num_matches_to_consider; ++i) {
        DMatch m = matches.at(i);
        total_squared_distance += m.distance*m.distance;
      }
      float normalized_distance = sqrt(total_squared_distance) / sqrt(num_matches_to_consider);

      // If the distance is too small, display a blank screen
      if (normalized_distance <= significance_threshold && normalized_distance > -1) {
        current_frame = Mat(current_frame.rows, current_frame.cols, current_frame.type(),
                       Scalar(0, 0, 0));
      }
      
      // Draw features
      drawKeypoints(current_frame, current_keypoints, features, Scalar(0, 0, 255));

      // Write distance
      std::stringstream ss;
      ss << normalized_distance;
      std::string s(ss.str());
      putText(features, s, Point(5,105), FONT_HERSHEY_SIMPLEX, 4, Scalar(255, 0, 255));

      // Display it!
      resize(features, display, Size(0,0), 2, 2);
      imshow("features", display);

      if(waitKey(30) >= 0) break;
    }
  
  return 0;
}

