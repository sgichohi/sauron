#include "FileMgr.h"
#include <dirent.h>
#include <string>

using namespace std;

/*******************************************************************************/
/* HELPER CLASS ENTRY                                                          */
/*******************************************************************************/
namespace COS518 {    
    class Entry {
        // Private members
        private:
        
        long time;
        long delta;
        std::string fileName;
        
        // Public members
        public:
        
        // Constructors
        Entry(long time, long delta, std::string fileName) {
            this->time     = time;
            this->delta    = delta;
            this->fileName = fileName;
        }
        
        // Public getters
        long getTime()     { return time;     }
        long getDelta()    { return delta;    }
        std::string getFileName() { return fileName; }
        
        // Make it comparable
        bool operator==(const Entry& that) const { return delta == that.delta; }
        bool operator<=(const Entry& that) const { return delta <= that.delta; }
        bool operator>=(const Entry& that) const { return delta >= that.delta; }
        bool operator!=(const Entry& that) const { return delta != that.delta; }
        bool operator< (const Entry& that) const { return delta <  that.delta; }
        bool operator> (const Entry& that) const { return delta >  that.delta; }
    };
}

std::ostream& operator<<(std::ostream &strm, COS518::Entry &e) {
    return strm << "Entry(" << e.getTime() << ", " << e.getDelta()
                << ", " << e.getFileName() << ")\n";
}

/*******************************************************************************/
/* FUNCTIONALITY FOR FILEMGR                                                   */
/*******************************************************************************/
namespace COS518 {
    // Parse a cstring name into an entry object.  Entry object must be freed.
    Entry FileMgr::parseName(const char *c) throw(int) {
        string s(c);
        
        size_t split = s.find("-");
        if (split == string::npos) throw 12;
        
        size_t ext = s.find(".");
        if (ext == string::npos || ext <= split) throw 12;
        
        string time = s.substr(0, split);
        string delta = s.substr(split + 1, ext - split - 1);
        
        try {
            return Entry(stol(time), stol(delta), s);
        } catch (...) {
            throw 12;
        }
    }
    
    // Insert into the pq
    void FileMgr::insert(string fileName) {
        lock->lock();
        try { pq->push(parseName(fileName.c_str())); }
        catch (int exp) { }
        lock->unlock();
    }
    
    // Remove the maximum element and return it
    string FileMgr::removeMax() throw(int) {
        lock->lock();
        
        if (pq->empty()) {
            lock->unlock();
            throw 13;
        }
        
        Entry e = pq->top();
        pq->pop();
        
        lock->unlock();
        return e.getFileName();
    }
    
    // Check whether the pq is empty
    bool FileMgr::isEmpty() {
        lock->lock();
        bool b = pq->empty();
        lock->unlock();
        return b;
    }
    
    int FileMgr::size() { return pq->size(); }

    // Constructor
    FileMgr::FileMgr(string directory) throw() {
        pq   = new priority_queue<Entry>();
        lock = new mutex();
        
        // Acquire the lock so initialization is threadsafe
        lock->lock();
        
        // Retrieve all files from the directory
        DIR *dp;
        struct dirent *ep;
        const char *c_dir = ("./" + directory).c_str();
        
        // Add all properly named files as entries in the priority queue
        if ((dp = opendir(c_dir))) {
            while ((ep = readdir(dp))) {
                try { pq->push(parseName(ep->d_name)); }
                catch(int exp) { }
            }
            
            closedir(dp);
        }
        
        // Release the mutex
        lock->unlock();       
    }
    
    // Destructor
    FileMgr::~FileMgr() {
        delete pq;
        delete lock;
    }
}
