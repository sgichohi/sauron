#ifndef USERDEFINED_H
#define USERDEFINED_H

namespace UserDefined {
    class Sendable {
        private:
        long timestamp;
        long delta;
        char *pic;
        long len;
        
        public:
        // Creates a new char[] with the object serialized; client must free the char *
        char  *serialize(int *outLength);
        
        // Creates a new sendable from the provided information.  Copies the char* into
        // a new buffer.  len is the length of the char * provided.
        Sendable(char *, long len, long timestamp, long delta);
        
        // Creates a new sendable from the provided buffer created by a call to serialize.
        // Copies information out of the char*.  len is the length of the char*
        Sendable(char *);
        
       ~Sendable();
        long score();
    };
    
    class Transformer {
        private:
        Sendable *cur;
        
        public:
        // Initialize the iterator
        void begin(char *pic, int len, long timestamp);
        
        // A test to see whether the iteration is finished
        bool finished();
        
        // Move the iterator to the next value
        void next();
        
        // Returns the current item in the iterator.  Iterator is responsible for freeing it.
        Sendable *current();
        
        // Constructor/Destructor for any datastructures the transformer needs
        Transformer();
       ~Transformer();
    };
}
#endif
