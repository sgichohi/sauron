#ifndef SENDABLEMAT_H
#define SENDABLEMAT_H

#include "../UserInterface.h"

#include <opencv2/opencv.hpp>

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
}

#endif
