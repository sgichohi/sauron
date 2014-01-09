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
        condition_variable *queue_empty;
        mutex *heapLock;
        mutex *queueLock;
        mutex *acksLock;
        priority_queue<HeapEntry> *pq;
        queue<QueueEntry>         *q;
        map<long, string>         *acks;
        
        HeapEntry parseName(string& dir, string& filename) throw(int);
        HeapEntry removeMax();
        
        // Public members
        public:
        FileMgr(string dir) throw();
       ~FileMgr();
       
        char *nextToAcks(long *ts, int *len, string&);
        void  ack(long ts);
        int   ackSize();
        
        void enqueue(long ts, char *buf, int len, string filename);
        void nextToQueue();
        bool queueIsEmpty();
        int  queueSize();
        
        void insert(long, long, string&);
        bool heapIsEmpty();
        int heapSize();
    };

}
#endif
