#include "ByteQueue.h"

/*******************************************************************************/
/* THREADSAFE QUEUE OF (BYTE, TIMESTAMP) PAIRS                                 */
/*******************************************************************************/
using namespace std;

namespace COS518 {
    struct QEntry {
        long timestamp;
        char *value;
        string path;
        int len;
    };

    // Constructor and destructor
    ByteQueue::ByteQueue() {
        lock = new mutex();
        q = new queue<QEntry>();
    }
    
    ByteQueue::~ByteQueue() {
        delete lock;
        delete q;
    }
    
    // Insert and remove
    void  ByteQueue::enqueue(long timestamp, char *b, string path, int len) {
        // Construct the entry
        QEntry entry;
        entry.timestamp = timestamp;
        entry.value = b;
        entry.path = path;
        entry.len = len;
        
        // Add the entry to the queue
        lock->lock();
        q->push(entry);
        lock->unlock();
    }
    
    char *ByteQueue::dequeue(long *timestamp, string *path, int *len) throw(int) {
        if (!timestamp || !path || !len) throw 5;
        
        // Throw an exception if the queue is empty
        lock->lock();
        if (q->empty()) {
            lock->unlock();
            throw 6;
        }
        
        // Remove the front element
        QEntry entry = q->front();
        q->pop();
        lock->unlock();
        
        // Return value
        *timestamp = entry.timestamp;
        *path = entry.path;
        *len = entry.len;
        return entry.value;
    }
    
    bool ByteQueue::isEmpty() {
        bool b = false;
        lock->lock();
        b = q->empty();
        lock->unlock();
        return b;
    }         
    
    int ByteQueue::size() {
        int i = 0;
        lock->lock();
        i = q->size();
        lock->unlock();
        return i;
    }
}
