#include "UserDefined.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace cv;
using namespace std;

namespace UserDefined {
  /*******************************************************************************/
  /* SENDABLE CLASS                                                              */
  /*******************************************************************************/
  // Constructor
  Sendable::Sendable(Mat pic, long timestamp, long score) {
    this->score = score;
    this->timestamp = timestamp;

    // Note that this assignment does not copy the underlying array. Rather, it
    // just makes another reference to the same data. When the caller destructs
    // its local version of pic, this will be the only reference to the array.
    // tl;dr: this assignment is intantaneous and OpenCV manages memory for us.
    this->pic = pic;
  }
    
  // Deserializer constructor
  Sendable::Sendable(char *b) {
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
  char *Sendable::serialize(int *outLength) {
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
    
  long Sendable::getScore() { return score; }

  Mat Sendable::getPic() { return pic; }

  Sendable::~Sendable() {
    pic.release();
  }

  
  /*******************************************************************************/
  /* TRANSFORMER CLASS                                                           */
  /*******************************************************************************/
  // Constructor
  Transformer::Transformer() { cur = NULL; }

  // Destructor
  Transformer::~Transformer() { if (cur != NULL) delete cur; }
    
  // Initialize the iterator
  void Transformer::begin(Mat pic, long timestamp) {
    cur = NULL;
        
    if (timestamp < 0) return;
        
    cur = new Sendable(pic, timestamp, timestamp);
  }
    
  // Check whether the iterator is finished
  bool Transformer::finished() { return (cur == NULL); }
    
  // Move the iterator to the next value
  void Transformer::next() {
    if (cur != NULL) delete cur;
    cur = NULL;
  }
    
  // Return the current item in the iterator
  Sendable *Transformer::current() { return cur; }
}
    
