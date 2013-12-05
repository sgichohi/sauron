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
    virtual void begin(cv::Mat pic, long timestamp) =0;
        
    // A test to see whether the iteration is finished
    virtual bool finished() =0;
        
    // Move the iterator to the next value
    virtual void next() =0;
        
    // Returns the current item in the iterator. Caller is responsible for freeing it.
    virtual Sendable *current() =0;
  };

  // UserFactory is a singleton object that allows code to get an object of the
  // user-defined Sendable or Transformer subclass, without knowing what the user
  // called it. The user should implement the non-static public methods to return
  // their own Sendable and Transformer implementations.

  // For example, to get an instance of the user's Sendable object, you call:
  // Sendable *mySendable = UserFactory::getFactory().getNewSendable();

  class UserFactory {
  private:
    UserFactory() {}; // Private, so that only one UserFactory can exist.
    UserFactory(UserFactory const&); // Prevents copy constructor, do not implement.
    void operator=(UserFactory const&); // Prevents assignment, do not implement.
    
  public:
    static UserFactory& getFactory()
    {
      static UserFactory instance;
      return instance;
    }

    Sendable *getNewSendable();
    Transformer *getNewTransformer();
  };
}
#endif
