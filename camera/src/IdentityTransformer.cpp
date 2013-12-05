#include "IdentityTransformer.h"
#include "UserDefined.h"

namespace UserDefined {
  IdentityTransformer::IdentityTransformer() { cur = NULL; }

  // Destructor
  IdentityTransformer::~IdentityTransformer() { if (cur != NULL) delete cur; }
    
  // Initialize the iterator
  void IdentityTransformer::begin(Sendable *sendable) { cur = sendable; }
    
  // Check whether the iterator is finished
  bool IdentityTransformer::finished() { return (cur == NULL); }
  
  // Move the iterator to the next value
  void IdentityTransformer::next() {
    if (cur != NULL) delete cur;
    cur = NULL;
  }

  Sendable *IdentityTransformer::current() { return cur; }
}
