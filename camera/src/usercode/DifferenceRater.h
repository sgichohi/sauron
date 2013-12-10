#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>

#include <math.h>

#include <algorithm>

namespace UserDefined {

  class DifferenceRater : public Transformer {
  private:
    bool full;
    SendableMat *cur;
    std::vector<cv::KeyPoint> cur_keypoints;
    cv::Mat cur_descriptors;

    float scale;
    int num_features;
    cv::ORB detector;
    
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
