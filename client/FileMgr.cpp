#include "FileMgr.h"
#include <dirent.h>
#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>

using namespace std;
using namespace UserDefined;

/*******************************************************************************/
/* FUNCTIONALITY FOR FILEMGR                                                   */
/*******************************************************************************/
namespace COS518 {
    // Parse a directory/filename pair into an entry object
    HeapEntry FileMgr::parseName(string& dir, string& filename) throw(int) {
        // Determine the file's sendable score
        size_t split  = filename.find("-");
        size_t split2 = filename.find(".");
        
        if (split == string::npos || split2 == string::npos) throw 12;
        long score = stol(filename.substr(split + 1, split2 - split - 1));
        long ts    = stol(filename.substr(0, split));
        
        return HeapEntry(ts, score, filename);        
    }
    
    /*******************************************************************************/
    /* MANIPULATE THE HEAP                                                         */
    /*******************************************************************************/
    // Insert into the pq
    void FileMgr::insert(long ts, long score, string &filename) {
        heapLock->lock();
        pq->push(HeapEntry(ts, score, filename));
        cerr << "HEAP: inserted " << filename << "\n";
        heapLock->unlock();
    }
    
    // Remove the maximum element and return it
    HeapEntry FileMgr::removeMax() throw(int) {
        heapLock->lock();
        
        if (pq->empty()) {
            heapLock->unlock();
            throw 13;
        }
        
        HeapEntry e = pq->top();
        pq->pop();
        
        heapLock->unlock();
        return e;
    }
    
    // Check whether the pq is empty
    bool FileMgr::heapIsEmpty() { return pq->empty(); }
    int  FileMgr::heapSize()    { return pq->size();  }
    
    /*******************************************************************************/
    /* MANIPULATE THE QUEUE                                                        */
    /*******************************************************************************/
    bool FileMgr::queueIsEmpty() { return q->empty(); }
    int  FileMgr::queueSize()    { return q->size();  }
    
    void FileMgr::enqueue(long ts, char *buf, int len, string filename) {
        queueLock->lock();
        QueueEntry qe(ts, buf, len, filename);
        q->push(qe);
        queueLock->unlock();
    }        
    
    void FileMgr::nextToQueue() throw(int) {
        // Get the HeapEntry
        HeapEntry he = removeMax();
        
        // Retrieve the path to the file
        string filename = he.getFilename();
        stringstream ss;
        ss << dir << "/" << filename;
        string path = ss.str();
        
        // Determine the length of the file
        ifstream is(path, ifstream::binary);
        if (!is) return;
        is.seekg(0, ios::end);
        int len = is.tellg();
        is.seekg(0, ios::beg);
        
        // Retrieve the file into a QueueEntry
        char *buf = new char[len];
        is.read(buf, len);
        is.close();
        
        // Finish
        assert(buf);
        enqueue(he.getTimestamp(), buf, len, filename);
        cerr << "QUEUE: added " << filename << "\n";
    }
    
    /*******************************************************************************/
    /* MANIPULATE THE MAP                                                          */
    /*******************************************************************************/
    char *FileMgr::nextToAcks(long *ts, int *len, string &filename) throw(int) {
        // Reserve the lock
        queueLock->lock();
        char *out;
        
        // Throw an exception if the queue is empty
        if (q->empty()) {
            queueLock->unlock();
            throw 12;
        }
        
        // Take the entry out of the queue
        QueueEntry qe = q->front();
        q->pop();
        
        // Prepare the output information
        *ts = qe.getTimestamp();
        out = qe.getBuffer(len);
        filename = qe.getFilename();
        
        // Unlock the lock
        queueLock->unlock();
        
        // Add to the ack map
        acksLock->lock();
        acks->insert(make_pair(*ts, filename));
        acksLock->unlock();
        
        return out;
    }
    
    void FileMgr::ack(long ts) {
        acksLock->lock();
        try {
            string path = dir + "/" + acks->at(ts);
            acks->erase(ts);
            acksLock->unlock();
            
            remove(path.c_str());
        } catch (...) { acksLock->unlock(); }
    }
    
    int FileMgr::ackSize() { return acks->size(); }
    
    /*******************************************************************************/
    /* CONSTRUCTOR                                                                 */
    /*******************************************************************************/
    FileMgr::FileMgr(string directory) throw() {
        acks = new map<long, string>;
        q    = new queue<QueueEntry>();
        pq   = new priority_queue<HeapEntry>();
        
        heapLock  = new mutex();
        queueLock = new mutex();
        acksLock  = new mutex();
        dir = directory;
        
        heapLock->lock();
        
        // Retrieve all files from the directory
        DIR *dp;
        struct dirent *ep;
        const char *c_dir = ("./" + dir).c_str();
        
        // Add all properly named files as entries in the priority queue
        if ((dp = opendir(c_dir))) {
            while ((ep = readdir(dp))) {
                try {
                    string filename(ep->d_name);
                    pq->push(parseName(dir, filename));
                } catch (...) { continue; }
            }
            
            closedir(dp);
        }
        
        heapLock->unlock(); 
    }
    
    // Destructor
    FileMgr::~FileMgr() {
        delete pq;
        delete q;
        delete heapLock;
        delete queueLock;
    }
}
