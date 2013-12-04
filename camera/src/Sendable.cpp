#include "UserDefined.h"

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
      std::stringstream ss;
      ss.str(b);

      size_t elem_size, elem_type;
      int flags, rows, cols;

      ss >> elem_size;
      ss >> elem_type;
      ss >> flags;
      ss >> rows;
      ss >> cols;
      
      std::string data = ss.string();

      
    }
    
    // Serializer
    char *Sendable::serialize(int *outLength) {
      std::stringstream ss;

      size_t elem_size = pic.elemSize();
      size_t elem_type = pic.type();
      std::streamsize data_size = elem_size * pic.rows * pic.cols;

      ss << elem_size << elem_type << pic.flags << pic.rows << pic.cols;
      ss.write(pic.data, data_size);

      std::string serialized = ss.str();
      char *cstr = malloc(serialized.size() + 1);
      if (NULL == cstr) {
        return NULL;
      }

      memcpy(cstr, serialized.c_str(), serialized.size());
      cstr[serialized.size()] = '\0';

      return cstr;
    }
    
    // Score
    long Sendable::score() { return score; }
    
    // Destructor
    Sendable::~Sendable() {
      pic.release();
    }
}
