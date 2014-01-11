#include "DoNothing.h"
#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <string.h>

#include <iostream>

using namespace cv;
using namespace std;

namespace UserDefined {
  
  /*******************************/
  /* USER FACTORY IMPLEMENTATION */
  /*******************************/

  Sendable *UserFactory::getNewSendable() {
    return new SendableMat();
  }

  Transformer *UserFactory::getNewTransformer() {
    return new IdentityTransformer();
  }

  /***************************************/
  /* IDENTITY TRANSFORMER IMPLEMENTATION */
  /***************************************/

  IdentityTransformer::IdentityTransformer() { cur = NULL; }

  // Destructor
  IdentityTransformer::~IdentityTransformer() { if (cur != NULL) delete cur; }
    
  // Initialize the iterator
  void IdentityTransformer::begin(Mat pic, long timestamp) {
    SendableMat *sendable = new SendableMat();
    sendable->initialize(pic, timestamp, timestamp);
    cur = sendable;
  }
    
  // Check whether the iterator is finished
  bool IdentityTransformer::finished() { return (cur == NULL); }
  
  // Move the iterator to the next value
  void IdentityTransformer::next() {
    if (cur != NULL) delete cur;
    cur = NULL;
  }

  Sendable *IdentityTransformer::current() { return cur; }
}
