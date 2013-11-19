/* Header file for ByteQueue.cpp */

#include <mutex>
#include <queue>

#ifndef BYTEQUEUE_H
#define BYTEQUEUE_H

namespace COS518 {
    struct QEntry;
    
    class ByteQueue {
        // Private members
        private:
        
        std::mutex *lock;
        std::queue<QEntry> *q;
        
        // Public members
        public:
        ByteQueue();
       ~ByteQueue();
        void  enqueue(long, char *, std::string, int);
        char *dequeue(long *, std::string *, int *) throw(int);
        bool  isEmpty();
        int   size();
    };
} 
#endif
