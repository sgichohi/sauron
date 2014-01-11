/* Header file for FileMgr class */

#ifndef FILEMGR_H
#define FILEMGR_H

#include "UserInterface.h"
#include "Entry.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <map>
#include <cstdio>
#include <condition_variable>

using namespace UserDefined;
using namespace std;

namespace COS518 {
    class FileMgr {
        // Private members
        private:
        string dir;
        condition_variable *heap_empty;
        mutex *heapLock;
        priority_queue<HeapEntry> *pq;
        
        HeapEntry parseName(string& dir, string& filename) throw(int);
        HeapEntry removeMax();
        
        // Public members
        public:
        FileMgr(string dir) throw();
       ~FileMgr();
        
        void insert(long, long, string&);
        bool heapIsEmpty();
        int heapSize();
        string getHeapMax();
    };

}
#endif
