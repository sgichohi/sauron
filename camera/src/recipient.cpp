#include "ClientSocket.h"
#include "usercode/SendableMat.h"
#include "ThreadPool.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;
using namespace cv;
using namespace UserDefined;
using namespace COS518;

/*********************************************************************/
/* RECEIVE SENDABLEMATS IN A SIMPLE, STRAIGHTFORWARD FASION          */
/*********************************************************************/

// Helper struct for items stored on the waiting queue
struct QueueEntry {
    long ts;
    long id;
    char *b;
};

int main(int argc, char **argv) {
    // Validate command line arguments
    if (argc < 3) {
        cerr << "Usage: server [camera address] [camera port]\n";
        cerr << "Options: [-b OR --benchmark] [-v OR --verbose] [-m OR --max INT] [-s OR --skip INT]\n";
        return 1;
    }

    // Handle options
    long max = 0;
    long skip = 0;
    bool verbose = false;
    bool benchmark = false;

    for (int i = 3; i < argc; i++) {
        string str(argv[i]);

        if (str == "-m" || str == "--max")  max  = stol(argv[++i]);
        if (str == "-s" || str == "--skip") skip = stol(argv[++i]);
        if (str == "-v" || str == "--verbose") verbose = true;
        if (str == "-b" || str == "--benchmark") benchmark = true;
    }

    if (verbose) { cerr << "max: " << max << "\nskip: " << skip << "\n"; }
    
    // A queue and a lock for storing received bytearrays
    queue<QueueEntry> *q = new queue<QueueEntry>();
    mutex *lock = new mutex();
    condition_variable has_item;
    bool stop = false;

    // A worker thread for storing sendables to disk
    function<void()> worker = [&q, &lock, &has_item, &stop]() {
        for (;;) {
            // Check whether there are any entries in the queue and wait on the cv
            unique_lock<mutex> ul(*lock);
            has_item.wait(ul, [&q, &stop]() { return !q->empty() || stop; });
            if (q->empty()) { ul.unlock(); return; }

            // Receive a char* from the queue
            QueueEntry qe = q->front();
            q->pop();
            ul.unlock();
            
            // Create a sendable from the char*
            SendableMat *sendable = new SendableMat();
            sendable->initialize(qe.b);
            delete qe.b;
            
            // Extract the picture from the sendable
            Mat pic = sendable->getPic();
         
            // Build a filename for the picture
            stringstream ss;
            ss << qe.id << "/" << qe.ts << "-" << sendable->getScore() << ".jpg";
            string filename = ss.str();
            
            // Save the picture to disk
            delete sendable;
            imwrite(filename, pic);
        }           
    };
    
    // Create a threadpool of workers
    ThreadPool tp(worker, 30);
    
    // Loop forever pulling sendables
    for (; ;) {
        // Initiate a connection
        if (verbose) cerr << "Initiating connection...\n";
        ClientSocket *s = new ClientSocket(argv[1], argv[2]);
        
        // If the connection can't be set up, retry once every second
        while (!s->isOpen()) {
            delete s;
            s = new ClientSocket(argv[1], argv[2]);
	    this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        // Get the length of a long
        char long_len;
        s->recv(&long_len, 1);
        if (verbose) cerr << "long_len: " << int(long_len) << "\n";
        
        // Get the camera's id number
        long id = s->recv();
        if (verbose) cerr << "id: " << id << "\n";
        
        // Create a directory if it doesn't already exist
        stringstream sss;
        sss  << id;
        mkdir(sss.str().c_str(), 0755);
        
        // Benchmarking metatdata
        long start = -1, sent = 0, oldbmark = 0;

        // Receive sendables
        while(s->isOpen()) {
            long ts, len;
            
            // Receive timestamp and length of the message
            try {
                ts = s->recv();
                len = s->recv();
            } catch (...) { continue; }
            
            // Receive the serialized sendable itself
            if (verbose) cerr << "received: " << len << "bytes\n";
            char *b = new char[len];
            try { s->recv(b, len); } catch (...) { continue; }
            
            // Create a new QueueEntry
            QueueEntry qe;
            qe.b = b;
            qe.ts = ts;
            qe.id = id;
            
            // Add the QueueEntry to the queue
            unique_lock<mutex> ul(*lock);
            q->push(qe);
            ul.unlock();
            has_item.notify_one();
            
            // Send the ack
            try { s->send(ts); } catch (...) { continue; }
            sent++;
            
            // Benchmark
            if (benchmark && sent == skip) {
                start = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
                oldbmark = start;
            } else if (benchmark && sent > skip) {
                long  bmark  = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
                cerr << "Average ms per Sendable: " << ((bmark - start)/(sent - skip)) << "\n";
                cerr << "Time for this Sendable: " << (bmark - oldbmark) << "\n";
                oldbmark = bmark;
            }

            // Exit if the maximum number of sendables has been hit
            if (max != 0 && sent == max) {
                stop = true;
                has_item.notify_all();
                tp.join(); 
                s->close();
                exit(0);
            }
        }
        
        // Clean up
        delete q;
        delete lock;
    }
}
