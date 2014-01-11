#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <algorithm>

namespace UserDefined {
  class FaceDetector : public Transformer {
  private:
    std::vector<SendableMat*> curs;
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
