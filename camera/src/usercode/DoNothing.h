#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>

namespace UserDefined {
  class IdentityTransformer : public Transformer {
  private:
    Sendable *cur;
    
  public:
    IdentityTransformer();
    ~IdentityTransformer();

    void begin(cv::Mat pic, long timestamp);
    bool finished();
    void next();
    Sendable *current();
  };
}

#endif
