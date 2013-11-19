/* Header file for AckSet.cpp */

#include <mutex>
#include <map>

#ifndef ACKSET_H
#define ACKSET_H

namespace COS518 {
    class AckSet {
        // Private members
        private:
        
        std::mutex *lock;
        std::map<long, std::string> *mp;
        
        // Public members
        public:
        AckSet();
       ~AckSet();
        void  insert(long, std::string);
        void  erase(long);
        int   size();
    };
} 
#endif
