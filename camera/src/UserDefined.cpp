#include "UserDefined.h"
#include <cstring>

// DEAD - This class is obsolete.

namespace UserDefined {
  /*******************************************************************************/
  /* SENDABLE CLASS                                                              */
  /*******************************************************************************/
  // Constructor
  Sendable::Sendable(Mat pic, long timestamp, long delta) {
    this->delta = delta;
    this->timestamp = timestamp;

    // Note that this assignment does not copy the underlying array. Rather, it
    // just makes another reference to the same data. When the caller destructs
    // its local version of pic, this will be the only reference to the array.
    // tl;dr: this assignment is intantaneous and OpenCV manages memory for us.
    this->pic = pic;
  }
    
  // Deserializer constructor
  Sendable::Sendable(char *b) {
    memcpy(&len,       b,                    sizeof(long));
    memcpy(&timestamp, b + sizeof(long),     sizeof(long));
    memcpy(&delta,     b + sizeof(long) * 2, sizeof(long));
        
    pic = new char[len];
    memcpy(pic,        b + sizeof(long) * 3, len);
  }
    
  // Serializer
  char *Sendable::serialize(size_t *outLength) {
    *outLength = len + sizeof(long) * 3;
    char *b = new char[*outLength];
        
    memcpy(b                   , &len,       sizeof(long));
    memcpy(b + sizeof(long),     &timestamp, sizeof(long));
    memcpy(b + sizeof(long) * 2, &delta,     sizeof(long));
    memcpy(b + sizeof(long) * 3, pic,        len);
        
    return b;
  }
    
  // Score
  long Sendable::getScore() { return delta; }

  // Destructor
  Sendable::~Sendable() {
    delete pic;
  }
  /*******************************************************************************/
  /* TRANSFORMER CLASS                                                           */
  /*******************************************************************************/
  // Constructor
  Transformer::Transformer() { cur = NULL; }
    
  // Destructor
  Transformer::~Transformer() { if (cur != NULL) delete cur; }
    
  // Initialize the iterator
  void Transformer::begin(char *pic, int len, long timestamp) {
    cur = NULL;
        
    if (pic == NULL || len == 0 || timestamp < 0) return;
        
    cur = new Sendable(pic, len, timestamp, timestamp);
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
