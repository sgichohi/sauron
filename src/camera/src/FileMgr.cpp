#include "FileMgr.h"
#include <dirent.h>
#include <string>
#include <sstream>
#include <fstream>
#include <assert.h>
#define HEAP_DEBUG 0

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
        // Add the item to the heap
        heapLock->lock();
        pq->push(HeapEntry(ts, score, filename));

        // Release the lock and notify the cv
        heapLock->unlock();
        heap_empty->notify_one();
    }
    
    // Remove the maximum element and return it
    HeapEntry FileMgr::removeMax() {
        // Ensure that there is something in the heap
        unique_lock<mutex> ul(*heapLock);
        while(pq->empty()) heap_empty->wait(ul);
        
        // Retrieve the maximum element
        HeapEntry e = pq->top();
        pq->pop();
        
        // Release the lock and finish
        ul.unlock();
        return e;
    }
    
    string FileMgr::getHeapMax() {
        return dir + "/" + removeMax().getFilename();
    }
    
    // Check whether the pq is empty
    bool FileMgr::heapIsEmpty() { return pq->empty(); }
    int  FileMgr::heapSize()    { return pq->size();  }
    
    /*******************************************************************************/
    /* CONSTRUCTOR                                                                 */
    /*******************************************************************************/
    FileMgr::FileMgr(string directory) throw() {
        pq   = new priority_queue<HeapEntry>();

        heap_empty  = new condition_variable();
        
        heapLock  = new mutex();
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
        delete heapLock;
        delete heap_empty;
    }
}
