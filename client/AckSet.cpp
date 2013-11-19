#include "AckSet.h"
#include <cstdio>

/*******************************************************************************/
/* THREADSAFE SET OF (NAME, TIMESTAMP) PAIRS                                   */
/*******************************************************************************/
using namespace std;

namespace COS518 {
    AckSet::AckSet() {
        lock = new mutex();
        mp = new map<long, string>();
    }
    
    AckSet::~AckSet() {
        delete lock;
        delete mp;
    }
    
    
    void  AckSet::insert(long ts, string name) {
        lock->lock();
        (*mp)[ts] = name;
        lock->unlock();
    }
    
    void  AckSet::erase(long ts) {
        lock->lock();
        string s;
        try {
            s = mp->at(ts);
            mp->erase(ts);
        } catch (...) {
            lock->unlock();
        }
        remove(s.c_str());
        lock->unlock();
    }
    
    int AckSet::size() {
        lock->lock();
        int sz = mp->size();
        lock->unlock();
        return sz;
    }
}
