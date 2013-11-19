#include "FileMgr.h"
#include "ByteQueue.h"
#include "AckSet.h"
#include "ClientSocket.h"
#include <thread>
#include <chrono>
#include <istream>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>

using namespace std;

namespace COS518 {

    // A function that never terminates, capturing frames from the webcam, adding
    // them to the priority queue, rinsing, and repeating.
    void captureThread(string directory, FileMgr *fm) {
        //for (; ;) {
            // Retrieve picture
            
            // Calculate long delta score
            
            //string fileName = "[milliseconds]-[delta-score].jpg";
            
            // Save picture as "[directory]/[milliseconds since 1/1/1970]-[delta score].jpg"
            
            //fm->insert(fileName);
        //}
    }
    
    // Wait for acknowledgements from the server
    void ackThread(AckSet *s, ClientSocket *sock) {
        for (; ;) {
            try {
                long l = sock->recv();
                cerr << "Acknowledgement for timestamp " << l << "\n";
                //s->erase(l);
            } catch (...) { return; }
        }
    }

    // Send the picture file in the specified directory with the specified filename to
    // the server.  When acknowledged, delete the file and return.
    void sendThread(ByteQueue *q, AckSet *s, int maxUnacked, ClientSocket *sock, int id) {        
        long time;
        char *buf;
        int len;
        string path;
        
        try {
            cout << "sending id\n";
            sock->send((long)id);
            cout << "id sent\n";
        } catch(...) { return; }
        
        for (; ;) {
            // If we already have enough items outstanding, sleep
            if (s->size() >= maxUnacked) {
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                continue;
            }
            
            // Send another picture
            try { 
                buf = q->dequeue(&time, &path, &len);
                s->insert(time, path);
            } catch (...) { continue; }
            
            cout << "Sending " << path << ", " << time << "\n";
            
            try {
                sock->send(time);
                sock->send((long)len);
                sock->send(buf, len);
            } catch (...) { q->enqueue(time, buf, path, len); return; }
            
            delete buf;
        }
    }
    
    // A function that loads a picture from a file into the sending queue
    enum { NOT_IN_USE, IN_FLIGHT, FINISHED};
    void loadThread(string dir, string fileName, ByteQueue *q, int *status) {
        string path = dir + "/" + fileName;
        
        // Determine the file's timestamp
        size_t split = fileName.find("-");
        if (split == string::npos) return;
        
        string time = fileName.substr(0, split);
        long ts = 0;
        
        try { ts = stol(time); } catch (...) { *status = FINISHED; return; }
                
        // Read the file into an array
        ifstream is(path, ifstream::binary);
        if (!is) {
            cerr << "Error : " << path << " " << strerror(errno) << "\n";
            *status = FINISHED;
            return;
        }
        
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        
        char* buf = new char[length];
        is.read(buf, length);
        
        // clean up
        is.close();
        
        q->enqueue(ts, buf, path, length);
        cout << "successfully loaded " << fileName << "\n"; 
        *status = FINISHED;   
    }

    // A function that never terminates, popping the maximum off of the queue and
    //  it in a separate thread.
    void loadMaster(string dir, int maxInFlight, int maxInQ, FileMgr *fileMgr, ByteQueue *q) {
        // Initialize thread-tracking information
        thread ts[maxInFlight];
        int inFlight = 0;
        
        int status[maxInFlight];
        for (int i = 0; i < maxInFlight; i++) status[i] = NOT_IN_USE;
        
        // Begin the main part of the function
        for (; ;) {
            // If we aren't at the maximum, transmit pictures until we are
            if (inFlight < maxInFlight && q->size() < maxInQ) {
            
                // Attempt to insert a new thread at each empty spot in the pool
                for (int i = 0; i < maxInFlight && inFlight < maxInFlight && q->size() < maxInQ; i++) {
                
                    // Keep trying to fill the spot until the heap has more information
                    while (status[i] == NOT_IN_USE) {
                        try {
                            string next = fileMgr->removeMax(); // Throws if heap is empty
                            status[i] = IN_FLIGHT;
                            ts[i] = thread(loadThread, dir, next, q, status + i);
                            cout << "Creating thread to load " << next << "\n";
                            inFlight++;
                        } catch (int exp) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(30));
                        }
                    }
                }
            // Otherwise, try to reap children
            } else {
                for (int i = 0; i < maxInFlight; i++) {
                    if (status[i] == FINISHED) {
                        status[i] = NOT_IN_USE;
                        ts[i].join();
                        inFlight--;
                    }
                }
            }       
        }
    }
}

using namespace COS518;

int main(int argc, char** argv) {
  if (argc != 5) {
      cerr << "Usage: client [Picture Directory] [Server URL] [Server Port] [Unique ID]\n";
      return 1;
  }

  string dir(argv[1]);
  FileMgr *fm = new FileMgr(dir);
  ByteQueue *q = new ByteQueue();
  AckSet *s = new AckSet();
  ClientSocket *sock = new ClientSocket(argv[3], argv[4]);
  
  thread load(loadMaster, dir, 2, 2, fm, q);
  thread capt(captureThread, dir, fm);
  
  while (true) {
      thread send(sendThread, q, s, 4, sock, stoi(string(argv[2])));
      thread acks(ackThread, s, sock);
      send.join();
      acks.join();
      
      while (!sock->isOpen()) {
          sock->close();
          delete sock;
          sock = new ClientSocket(argv[3], argv[4]);
          cerr << "Socket is recovering\n";
      }
  }
      
  load.join();
  capt.join();
  return 0;
}
