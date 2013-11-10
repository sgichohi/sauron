/* Header file for FileMgr class */

#ifndef FILEMGR_H
#define FILEMGR_H

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

namespace COS518 {

    class Entry;

    class FileMgr {
        // Private members
        private:
        
        std::mutex *lock;
        std::priority_queue<Entry> *pq;
        
        Entry parseName(const char *) throw(int);
        
        // Public members
        public:
        FileMgr(std::string directory) throw();
        ~FileMgr();
        void insert(std::string);
        std::string removeMax() throw(int);
        bool isEmpty();
    };

}
#endif
