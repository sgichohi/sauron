#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "UserInterface.h"

#include <math.h>
#include <opencv/cv.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <algorithm>

namespace UserDefined {
  class SendableMats : public Sendable {
  private:
  std::vector<cv::Mat> pics;

  public:
    void initialize(std::vector<cv::Mat> pics, long timestamp, long score);
    
    // Deserializer constructor
    void initialize(char *b);
    
    // Serializer
    char *serialize(int *outLength);
    
    const std::vector<cv::Mat> getPics();

    SendableMats();
    ~SendableMats();
  };

  class FaceDetector : public Transformer {
  private:
    SendableMats *cur;
    bool full;
    cv::CascadeClassifier face_cascade;
    
  public:
    FaceDetector();
    ~FaceDetector();

    void begin(cv::Mat pic, long timestamp);
    bool finished();
    void next();
    Sendable *current();
  };
}

#endif
