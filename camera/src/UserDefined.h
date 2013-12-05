#ifndef USERDEFINED_H
#define USERDEFINED_H

#include <opencv/cv.h>

namespace UserDefined {

  class Sendable {
  public:
    // Creates a new char[] with the object serialized; client must free the char *
    virtual char *serialize(int *outLength) =0;

    Sendable() {};
    virtual ~Sendable() {};

    // Creates a new sendable from the provided information.  Copies the char* into
    // a new buffer.  len is the length of the char * provided.
    virtual void initialize(cv::Mat pic, long timestamp, long score) =0;
        
    // creates a new sendable from the provided buffer created by a call to serialize.
    // Copies information out of the char*.  len is the length of the char*
    virtual void initialize(char *) =0;
        
    long getScore() { return score; }
    long getTimestamp() { return timestamp; }

  protected:
    long score;
    long timestamp;
  };
    
  class Transformer {
  public:
    // Constructor/Destructor for any datastructures the transformer needs
    Transformer() {};
    virtual ~Transformer() {};

    // Initialize the iterator. Passes responsibility for the allocated memory
    // to the Transformer.
    virtual void begin(Sendable *sendable) =0;
        
    // A test to see whether the iteration is finished
    virtual bool finished() =0;
        
    // Move the iterator to the next value
    virtual void next() =0;
        
    // Returns the current item in the iterator. Caller is responsible for freeing it.
    virtual Sendable *current() =0;
  };
}
#endif
