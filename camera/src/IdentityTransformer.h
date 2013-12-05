#include "UserDefined.h"

namespace UserDefined {
  class IdentityTransformer : public Transformer {
  private:
    Sendable *cur;
    
  public:
    IdentityTransformer();
    ~IdentityTransformer();

    void begin(Sendable *sendable);
    bool finished();
    void next();
    Sendable *current();
  };
}
