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
    int inLength;
    memcpy(&inLength, b, sizeof(inLength));

    int picLen = inLength - (sizeof(inLength) + sizeof(timestamp) + sizeof(score));
    if (picLen < 0) {
      fprintf(stderr, "Input to deserializer too short.");
    }

    memcpy(&timestamp, b + sizeof(inLength), sizeof(timestamp));
    memcpy(&score, b + sizeof(inLength) + sizeof(timestamp), sizeof(score));

    vector<uchar> buf;
    buf.reserve(picLen);
    buf.assign(b + sizeof(inLength) + sizeof(timestamp) + sizeof(score),
               b + inLength);
      
    pic = imdecode(buf, 1);
  }
    
  // Serializer
  char *SendableMat::serialize(int *outLength) {
    vector<uchar> buf;
    imencode(".jpg", pic, buf);

    *outLength = sizeof(*outLength) + sizeof(timestamp) + sizeof(score) + buf.size();
    char *b = new char[*outLength];
    memcpy(b, outLength, sizeof(*outLength));
    memcpy(b + sizeof(*outLength), &timestamp, sizeof(timestamp));
    memcpy(b + sizeof(*outLength) + sizeof(timestamp), &score, sizeof(score));
    memcpy(b + sizeof(*outLength) + sizeof(timestamp) + sizeof(score),
           buf.data(), buf.size());
      
    return b;
  }
    
  const Mat SendableMat::getPic() { return pic; }

  SendableMat::SendableMat() {}
  SendableMat::~SendableMat() { pic.release(); }

}
