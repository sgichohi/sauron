#include "FileMgr.h"
#include "AbstractSocket.h"
#include "ServerSocket.h"
#include "UserInterface.h"
#include "ThreadPool.h"
#include "Util.h"

#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <opencv2/opencv.hpp>

bool verbose = false;

using namespace std;
using namespace UserDefined;
using namespace cv;

namespace COS518 {
  /*******************************************************************************/
  /* CAPTURE THREAD                                                              */
  /*-----------------------------------------------------------------------------*/
  /* Capture pictures from the webcam, calculate delta scores, save as a file    */
  /* and add an entry to the FileMgr for that picture.                           */
  /*******************************************************************************/
  long extendLamport(string timefile, long addend) {
    // Establish a timestamp
    char buf[100];
    memset(buf, 0, 100);
    long lamport;
    stringstream ss;
	    
    // Read timestamp file
    FILE* fp = fopen(timefile.c_str(), "rb");
    if (!fread(buf, 1, 100, fp));
    fclose(fp);
	    
    // Write timestamp file
    lamport = stol(buf);
    ss << (lamport + addend);
    fp = fopen(timefile.c_str(), "wb");
    fwrite(ss.str().c_str(), 1, ss.str().length(), fp);
    fclose(fp);
	    
    return lamport;
  }
	
  // A structure for storing the important information of each sendable
  struct queueInfo {
    int ts;
    int score;
    char *buf;
    int  len;
  };
    
  // The thread that actually performs the capturing
  void captureThread(string directory, FileMgr *fm, string timefile, int maxInQ, int max) {
    long lamport = extendLamport(timefile, 1000);
    long limit   = lamport + 1000;
    Transformer *trfm = UserFactory::getFactory().getNewTransformer();
        
    queue<queueInfo> *q = new queue<queueInfo>();
    mutex *lock = new mutex();
    condition_variable *cv = new condition_variable();

    // Declare a function for performing writing and enqueuing
    function<void()> worker = [&directory, &q, &lock, fm, maxInQ, &cv]() {
      for (; ;) {
        // Wait for the queue to have something in it
        unique_lock<mutex> ul(*lock);
        while (q->empty()) cv->wait(ul);
	
        // Process the next item on the queue
        queueInfo qi = q->front();
        q->pop();
        ul.unlock();
		            
        // Build the path to the file
        stringstream ss;
        ss << qi.ts << "-" << qi.score << ".sendable";
        string filename = ss.str();
        string path = directory + "/" + filename;
	        	                
        // Write the information to a file
        FILE *file = fopen(path.c_str(), "wb");
        fwrite(qi.buf, 1, qi.len, file);
        fclose(file);
        if (verbose) cerr << "CAPTURE: " << this_thread::get_id() << " ";
	           
	    // Insert into the heap
        fm->insert(qi.ts, qi.score, filename);
        if (verbose) cerr << "slow path " << filename << "\n";
        delete qi.buf;        
      }
    };
	                                 
    // A threadpool to manage workers
    ThreadPool tp(worker, 4);

    VideoCapture cap(0);
    if (!cap.isOpened()) {
      cerr << "Cannot access the webcam, VideoCapture initialization failed.\n";
    }
        
    Mat pic;
    long ts = 0;

    // Infinite loop for capturing pictures
    for (; ; lamport++) {
      // Capture a new picture
      cap >> pic;
      
      for (trfm->begin(pic, ts); !trfm->finished(); trfm->next()) {

        // Update the lamport time if necessary
        if (lamport >= limit) {
          lamport = extendLamport(timefile, 1000);
          limit   = lamport + 1000;
        }
		        
        // Create a queueInfo
        queueInfo qi;
        qi.ts = lamport;
        qi.score = trfm->current()->getScore();
        qi.buf = trfm->current()->serialize(&qi.len);
		        
        // Enqueue it
        lock->lock();
        q->push(qi);
        lock->unlock();
        cv->notify_one();
      }

      long  ts2  = now();
      long used = ts2 - ts;
      ts = ts2;
      cerr << "used: " << used << "\n";

      if (max > 0) {
          max--;
          if (max <= 0) {
              cerr << "Exiting...";
              this_thread::sleep_for(chrono::milliseconds(3000));
              exit(0);
          }
      }
    }
    delete q;
    delete lock;
    delete trfm;
    delete cv;
  }
    
  /*******************************************************************************/
  /* LOAD THREAD & LOAD MASTER                                                   */
  /*-----------------------------------------------------------------------------*/
  /* Load master produces worker threads that read pictures from disk into       */
  /* memory, adding them to the ByteQueue with their metadata. Each worker adds  */
  /* a single picture.                                                           */
  /*******************************************************************************/
  void loadThread(FileMgr *fm, AbstractSocket *sock, int id) {
    char c = id & 0xFF;
    
    // Send the id
    try {
      sock->send(&c, 1);
    } catch(...) { return; }
    
    // Send off of the heap
    for (; ;) {
      string str = fm->getHeapMax();
        
      try {
        sock->send((char *)str.c_str(), str.length() + 1);
      } catch (...) {
        return;
      }
        
      if (verbose) {
        cerr << "LOAD: " << str << " has loaded\n";
      }
    }
  }
}

using namespace COS518;

int main(int argc, char** argv) {
  // Return an error if an improper number of arguments are specified
  if (argc < 3) {
    cerr << "Usage: camera [Unique ID] [Server Port]\n";
    cerr << "Options: [-v OR --verbose] [-r OR --reconnect]\n";
    cerr << "[-d OR --directory {dir to store output}] [-m or --max {max number of sendables}]\n";
    return 1;
  }

  // Initialize filestructure
  string dir = "output";
  string cfg = "config";
  string timefile = cfg + "/time.txt";
  bool reconnect = false;
  int max = -1;

  // Handle command line flags
  for (int i = 2; i < argc; i++) {
    string str(argv[i]);
    if (str == "-v" || str == "--verbose") verbose = true; 
    if (str == "-r" || str == "--reconnect") reconnect = true;
    if (str == "-m" || str == "--max") max = stoi(argv[++i]);
    if (str == "-d" || str == "--directory") dir = string(argv[++i]);
  }  
  
  // Create directories as necessary
  mkdir(dir.c_str(), 0755);
  mkdir(cfg.c_str(), 0755);
    
  // Create the timefile
  FILE *fp;
  if ((fp = fopen(timefile.c_str(), "r"))) {
    fclose(fp);
  } else {
    ofstream fp(timefile, ios::binary);
    fp << "0";
    fp.close();
  }

  // Initialize datastructures
  FileMgr *fm = new FileMgr(dir);
  Acceptor acpt(argv[2]);
  if (verbose) {
    cerr << "MAIN: Waiting to accept connection \n";
  }
  ServerSocket *sock = acpt.accept();
  if (verbose) {
    cerr << "MAIN: Accepted connection\n";
  }
  
  // Begin threads that are never supposed to crash (capture)
  thread capt(captureThread, dir, fm, timefile, 30, max);
  
  // Begin threads that crash if the socket closes
  for (; ;) {
    // Handle a closed socket
    while (!sock->isOpen()) {
      delete sock;

      // Exit if desired
      if (!reconnect) exit(0);

      // Reconnect otherwise
      if (verbose) {
        cerr << "MAIN: Waiting for a connection\n";
        this_thread::sleep_for(chrono::milliseconds(1000));
      }
      sock = acpt.accept();
    }
      
    // Initialize the load thread
    thread load(loadThread, fm, sock, stoi(argv[1]));
      
    // Load crashes if the socket closes
    load.join();
  }
 
  return 1;
}
