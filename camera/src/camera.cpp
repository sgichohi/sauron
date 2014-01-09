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
  void captureThread(string directory, FileMgr *fm, string timefile, int maxInQ) {
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
        ss << qi.ts << "-" << qi.score << ".jpg";
        string filename = ss.str();
        string path = directory + "/" + filename;
	        	                
        // Write the information to a file
        FILE *file = fopen(path.c_str(), "wb");
        fwrite(qi.buf, 1, qi.len, file);
        fclose(file);
        if (verbose) cerr << "CAPTURE: " << this_thread::get_id() << " ";
	           
        // If the heap is empty, take the fast path
        if (fm->heapSize() == 0 && fm->queueSize() < maxInQ) {
          fm->enqueue(qi.ts, qi.buf, qi.len, filename);
          if (verbose) cerr << "fast path " << filename << "\n";
        }
	           
        // Otherwise, add the information to the heap
        else {
          fm->insert(qi.ts, qi.score, filename);
          if (verbose) cerr << "slow path " << filename << "\n";
          delete qi.buf;
        }
      }
    };
	                                 
    // A threadpool to manage workers
    ThreadPool tp(worker, 20);

    VideoCapture cap(0);
    if (!cap.isOpened()) {
      cerr << "Cannot access the webcam, VideoCapture initialization failed.\n";
    }
        
    Mat pic;

    // Infinite loop for capturing pictures
    for (; ; lamport++) {
      // Capture a new picture
      cap >> pic;
      long  ts  = now();
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
        cv->notify_one();
        lock->unlock();
      }

      if (verbose) this_thread::sleep_for(chrono::milliseconds(1000));
      long  ts2  = now();
      long used = ts2 - ts;
      cerr << "used: " << used << "\n";
    }
    delete q;
    delete lock;
    delete trfm;
    delete cv;
  }
    
  /*******************************************************************************/
  /* ACKNOWLEDGEMENT THREAD                                                      */
  /*-----------------------------------------------------------------------------*/
  /* Wait for acknowledgements from the server.  When received, delete the file  */
  /* that has been acknowledged and wait for another acknowledgement.  Crashes   */
  /* if the provided socket fails for any reason.                                */
  /*******************************************************************************/
  void ackThread(FileMgr *fm, AbstractSocket *sock) {
    for (; ;) {
      try {
        long l = sock->recv();
        if (verbose) { 
          cerr << "ACK: Acknowledgement received for timestamp " << l << "\n";
        }
        fm->ack(l);
      } catch (...) { return; }
    }
  }

  /*******************************************************************************/
  /* SEND THREAD                                                                 */
  /*-----------------------------------------------------------------------------*/
  /* Retrieve arrays of bytes with picture metadata from the ByteQueue and send  */
  /* them to the server out of the specified socket.  Once sent, the picture     */
  /* metadata is transferred into the AckSet.  Crashes if the provided socket    */
  /* fails for any reason.  If maxUnacked is <= 0, there is no cap on the number */
  /* of items that will be left unacked at once.                                 */
  /*******************************************************************************/
  void sendThread(FileMgr *fm, AbstractSocket *sock, int id) {        
    long ts;
    char *buf;
    int len;
    string filename;
        
    // Send the client's ID number to the server.  Crash the thread on failure.
    char long_size = (char) sizeof(long);
    try {
      sock->send(&long_size, 1);
      sock->send((long)id);
    } catch(...) { return; }
        
    // Loop forever sending items from the FileManager's queue
    for (; ;) {
            
      // Take a picture from the FileManager
      buf = fm->nextToAcks(&ts, &len, filename);
            
      // Attempt to send the item over the wire
      try {
        sock->send(ts);
        sock->send((long)len);
        sock->send(buf, len);
      }
      // On failure, add the items back into the queue and crash the thread
      catch (...) {
        fm->enqueue(ts, buf, len, filename);
        return;
      }
            
      // On success, add an entry to the AckSet and deallocate the buffer memory
      if (verbose) {
        cerr << "SEND: Picture " << ts << " successfully sent\n";
      }
      delete buf;
    }
  }
    
  /*******************************************************************************/
  /* LOAD THREAD & LOAD MASTER                                                   */
  /*-----------------------------------------------------------------------------*/
  /* Load master produces worker threads that read pictures from disk into       */
  /* memory, adding them to the ByteQueue with their metadata. Each worker adds  */
  /* a single picture.                                                           */
  /*******************************************************************************/
  void loadPool(FileMgr *fm, int maxInFlight, int maxInQ) {
    
    // Declare a lambda to load the queue
    function<void()> f = [fm, &maxInQ]() {
      // Sleep if the queue is currently full
      for (; ;) {
        if (fm->queueSize() < maxInQ) {
          fm->nextToQueue();
                        
          if (verbose) {
            cerr << "LOAD: " << this_thread::get_id() << " has loaded\n";
          }
        }
      }
    };
        
    // Create a threadpool for queueing
    ThreadPool tp(f, maxInFlight);
    tp.join();
  }
}

using namespace COS518;

int main(int argc, char** argv) {
  // Return an error if an improper number of arguments are specified
  if (argc < 3) {
    cerr << "Usage: camera [Unique ID] [Server Port]\n";
    cerr << "Options: [-v OR --verbose] [-r OR --reconnect]\n";
    return 1;
  }

  // Initialize filestructure
  string dir = "output";
  string cfg = "config";
  string timefile = cfg + "/time.txt";
  bool reconnect = false;

  // Handle command line flags
  for (int i = 2; i < argc; i++) {
    string str(argv[i]);
    if (str == "-v" || str == "--verbose") verbose = true; 
    if (str == "-r" || str == "--reconnect") reconnect = true;   
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
  
  // Begin threads that are never supposed to crash (capture and load)
  thread load(loadPool, fm, 30, 30);
  thread capt(captureThread, dir, fm, timefile, 200);
  
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
      
    // Initialize the sender and acknowledgement threads
    thread send(sendThread, fm, sock, stoi(string(argv[1])));
    thread acks(ackThread, fm, sock);
      
    // Both threads crash if the socket closes
    send.join();
    acks.join();
  }
  
  // Should never get here
  load.join();
  capt.join();
  return 1;
}
