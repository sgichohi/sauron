#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "UserInterface.h"

#include <math.h>
#include <opencv/cv.h>

#include <algorithm>

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
    
    const cv::Mat getPic();

    SendableMat();
    ~SendableMat();
  };

  class DifferenceRater : public Transformer {
  private:
    bool full;
    SendableMat *cur;
    std::vector<cv::KeyPoint> cur_keypoints;
    cv::Mat cur_descriptors;

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
