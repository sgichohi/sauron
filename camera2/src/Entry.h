#ifndef ENTRY_H
#define ENTRY_H

#include "UserInterface.h"
#include <iostream>

using namespace std;
using namespace UserDefined;

namespace COS518 {
    class HeapEntry {
        private:
        string filename;
        long score;
        long ts;
        
        public:
        HeapEntry(long ts, long score, const string&);
       ~HeapEntry();
       
        string getFilename();
        long   getScore();
        long   getTimestamp();
        
        bool operator<=(const HeapEntry& that) const;
        bool operator< (const HeapEntry& that) const;
    };
    
    class QueueEntry {
        private:
        string    filename;
        long      ts;
        char     *buf;
        int       len;
        
        public:
        QueueEntry(long ts, char *buf, int len, const string filename);
       ~QueueEntry();
       
        string getFilename();
        long   getTimestamp();
        char  *getBuffer(int *len);
    };
}

#endif
