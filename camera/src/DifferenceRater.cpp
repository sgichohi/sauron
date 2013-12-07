#include "DifferenceRater.h"
#include "UserInterface.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
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

  /*******************************/
  /* SENDABLE MAT IMPLEMENTATION */
  /*******************************/

  void SendableMat::initialize(Mat pic, long timestamp, long score) {
    this->score = score;
    this->timestamp = timestamp;

    // Note that this assignment does not copy the underlying array. Rather, it
    // just makes another reference to the same data. When the caller destructs
    // its local version of pic, this will be the only reference to the array.
    // tl;dr: this assignment is intantaneous and OpenCV manages memory for us.
    this->pic = pic;
  }
    
  // Deserializer constructor
  void SendableMat::initialize(char *b) {
    int inLength;
    memcpy(&inLength, b, sizeof(inLength));

    int picLen = inLength - (sizeof(inLength) + sizeof(timestamp) + sizeof(score));
    if (picLen < 0) {
      fprintf(stderr, "Input to deserializer too short.");
    }

    memcpy(&timestamp, b + sizeof(inLength), sizeof(timestamp));
    memcpy(&score, b + sizeof(inLength) + sizeof(timestamp), sizeof(score));

    vector<uchar> buf;
    buf.reserve(picLen);
    buf.assign(b + sizeof(inLength) + sizeof(timestamp) + sizeof(score),
               b + inLength);
      
    pic = imdecode(buf, 1);
  }
    
  // Serializer
  char *SendableMat::serialize(int *outLength) {
    vector<uchar> buf;
    imencode(".jpg", pic, buf);

    *outLength = sizeof(*outLength) + sizeof(timestamp) + sizeof(score) + buf.size();
    char *b = new char[*outLength];
    memcpy(b, outLength, sizeof(*outLength));
    memcpy(b + sizeof(*outLength), &timestamp, sizeof(timestamp));
    memcpy(b + sizeof(*outLength) + sizeof(timestamp), &score, sizeof(score));
    memcpy(b + sizeof(*outLength) + sizeof(timestamp) + sizeof(score),
           buf.data(), buf.size());
      
    return b;
  }
    
  const Mat SendableMat::getPic() { return pic; }

  SendableMat::SendableMat() {}
  SendableMat::~SendableMat() { pic.release(); }

  /***********************************/
  /* DIFFERENCE RATER IMPLEMENTATION */
  /***********************************/

  DifferenceRater::DifferenceRater() {
    cur = NULL;
    full = false;
    num_features = 1000;
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

      Mat gray_frame;
      cvtColor(pic, gray_frame, CV_BGR2GRAY);
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
      
      Mat gray_frame;
      cvtColor(pic, gray_frame, CV_BGR2GRAY);
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
      
      // Draw the keypoints, just for fun
      drawKeypoints(pic, cur_keypoints, pic, Scalar(0, 0, 255));

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
