#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "UserInterface.h"

#include <math.h>
#include <opencv/cv.h>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>

#include <algorithm>

using namespace cv;
using namespace std;

namespace UserDefined {
  class SendableMat : public Sendable {
  private:
    cv::Mat pic;

  public:
    void initialize(cv::Mat pic, long timestamp, long score);
    
    // Deserializer constructor
    void initialize(char *b);
    
    // Serializer
    char *serialize(int *outLength);
    
    cv::Mat getPic();

    SendableMat();
    ~SendableMat();
  };

  class DifferenceRater : public Transformer {
  private:
    bool full;
    SendableMat *cur;
    vector<KeyPoint> cur_keypoints;
    Mat cur_descriptors;

    int num_features;
    ORB detector;
    
  public:
    DifferenceRater();
    ~DifferenceRater();

    void begin(cv::Mat pic, long timestamp);
    bool finished();
    void next();
    Sendable *current();
  };
}

#endif
