#include "UserDefined.h"

#include <opencv/cv.h>

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
}
