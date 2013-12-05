#ifndef DIFFERENCERATER_H
#define DIFFERENCERATER_H

#include "UserInterface.h"
#include <opencv/cv.h>

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
}

#endif
