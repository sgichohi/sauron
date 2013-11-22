#include "Entry.h"

using namespace std;
using namespace UserDefined;

namespace COS518 {
    // Constructor
    HeapEntry::HeapEntry(long ts, long score, const string& filename) {
        this->ts       = ts;
        this->score    = score;
        this->filename = filename;
    }
    
    // Destructor
    HeapEntry::~HeapEntry() { }
    
    // Getters
    string HeapEntry::getFilename()  { return filename; }
    long   HeapEntry::getScore()     { return score;    }
    long   HeapEntry::getTimestamp() { return ts;       }
    
    // Comparison
    bool HeapEntry::operator<=(const HeapEntry& that) const {
        return score <= that.score;
    }
    bool HeapEntry::operator<(const HeapEntry& that) const {
        return score < that.score;
    }
    
    // Constructor
    QueueEntry::QueueEntry(long ts, char *buf, int len, const string filename) {
        this->ts = ts;
        this->buf = buf;
        this->filename = filename;
        this->len = len;
    }
    
    QueueEntry::~QueueEntry() { }
    
    string QueueEntry::getFilename()     { return filename;      }
    long   QueueEntry::getTimestamp()    { return ts;            }
    char  *QueueEntry::getBuffer(int *v) { *v = len; return buf; }
}
