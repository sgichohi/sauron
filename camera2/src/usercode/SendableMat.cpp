#include "SendableMat.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

namespace UserDefined {

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
    this->score = 0;
    this->timestamp = 0;
  }
    
  // Serializer
  char *SendableMat::serialize(int *outLength) {
    vector<uchar> buf;
    imencode(".jpg", pic, buf);

    *outLength = buf.size();
    char *b = new char[*outLength];
    memcpy(b, buf.data(), buf.size());
      
    return b;
  }
    
  const Mat SendableMat::getPic() { return pic; }

  SendableMat::SendableMat() {}
  SendableMat::~SendableMat() { pic.release(); }

}
