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
#include <sstream>

using namespace std;

namespace COS518 {
    /*******************************************************************************/
    /* CAPTURE THREAD                                                              */
    /*-----------------------------------------------------------------------------*/
    /* Capture pictures from the webcam, calculate delta scores, save as a file    */
    /* and add an entry to the FileMgr for that picture.                           */
    /*******************************************************************************/
    void captureThread(string directory, FileMgr *fm) {
    
        for (long i = 0; true; i++) {
            stringstream ss;
            ss << i << "-" << i << ".jpg";
            string filename = ss.str();
            string path = directory + "/" + filename;
            string content = "hello!";
            
            FILE *file = fopen(path.c_str(), "wb");
            fwrite(content.c_str(), 1, content.length(), file);
            fclose(file);
            
            fm->insert(filename);
            cerr << "CAPTURE: generated file " << filename << "\n";
            this_thread::sleep_for(chrono::milliseconds(50));  
        }
            
        //for (; ;) {
            // Retrieve picture
            
            // Calculate long delta score
            
            // string fileName = "[milliseconds]-[delta-score].jpg";
            
            // Save picture as "[directory]/[milliseconds since 1/1/1970]-[delta score].jpg"
            
            //fm->insert(fileName);
        //}
    }
    
    /*******************************************************************************/
    /* ACKNOWLEDGEMENT THREAD                                                      */
    /*-----------------------------------------------------------------------------*/
    /* Wait for acknowledgements from the server.  When received, delete the file  */
    /* that has been acknowledged and wait for another acknowledgement.  Crashes   */
    /* if the provided socket fails for any reason.                                */
    /*******************************************************************************/
    void ackThread(AckSet *s, ClientSocket *sock) {
        for (; ;) {
            try {
                long l = sock->recv();
                cerr << "ACK: Acknowledgement received for timestamp " << l << "\n";
                //s->erase(l);
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
    void sendThread(ByteQueue *q, AckSet *s, ClientSocket *sock, int id, int maxUnacked = 0) {        
        long ts;
        char *buf;
        int len;
        string path;
        
        // Send the client's ID number to the server.  Crash the thread on failure.
        try { sock->send((long)id); } catch(...) { return; }
        
        // Loop forever sending items from the ByteQueue
        for (; ;) {
            // If the maximum number of items is currently outstanding, sleep
            while (maxUnacked > 0 && s->size() >= maxUnacked) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            
            // Attempt to take a picture from the ByteQueue.
            while (true) {
                try { buf = q->dequeue(&ts, &path, &len); }
                
                // On failure, sleep and try again
                catch (...) {
                    this_thread::sleep_for(chrono::milliseconds(100));
                    continue;
                }
                
                // On success, exit the loop
                break;
            }
            
            // Attempt to send the item over the wire
            try {
                sock->send(ts);
                sock->send((long)len);
                sock->send(buf, len);
            }
            // On failure, add the items back into the queue and crash the thread
            catch (...) {
                q->enqueue(ts, buf, path, len);
                return;
            }
            
            // On success, add an entry to the AckSet and deallocate the buffer memory
            cout << "SEND: Picture " << ts << " successfully sent\n";
            s->insert(ts, path);
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
    
    // Enum for keeping track of thread state
    enum { NOT_IN_USE, IN_FLIGHT, FINISHED};
    
    // The worker thread
    void loadThread(string dir, string fileName, ByteQueue *q, int *status) {
        // Determine the file's timestamp from its filename
        size_t split = fileName.find("-");
        if (split == string::npos) { *status = FINISHED; return; }
        
        long ts = 0;
        try { ts = stol(fileName.substr(0, split)); }
        catch (...) { *status = FINISHED; return; }
                
        // Read the file into an array
        string path = dir + "/" + fileName;
        ifstream is(path, ifstream::binary);
        
        // Crash the thread if the file does not exist
        if (!is) {
            cerr << "Error : " << path << " " << strerror(errno) << "\n";
            *status = FINISHED;
            return;
        }
        
        // Determine the file length
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        
        // Read the file into a newly allocated buffer
        char* buf = new char[length];
        is.read(buf, length);
        
        // Clean up and finish
        is.close();
        q->enqueue(ts, buf, path, length);
        cout << "LOAD: successfully loaded timestamp " << ts << "\n"; 
        *status = FINISHED;   
    }

    // Manages a pool of loadThreads
    void loadMaster(string dir, FileMgr *fileMgr, ByteQueue *q, int maxInFlight, int maxInQ) {
        // Initialize thread-tracking information
        thread ts[maxInFlight];
        int inFlight = 0;
        int status[maxInFlight];
        for (int i = 0; i < maxInFlight; i++) status[i] = NOT_IN_USE;
        
        // Spin off worker threads
        for (; ;) {
            // Transmit as many pictures as possible
            for (int i = 0; i < maxInFlight; i++) {
                // Reap if finished
                if (status[i] == FINISHED) {
                    status[i] = NOT_IN_USE;
                    ts[i].join();
                    inFlight--;
                }
                
                // Place another thread in flight if possible
                while (status[i] == NOT_IN_USE) {
                    if (inFlight < maxInFlight && q->size() < maxInQ) {
                        try {
                            string next = fileMgr->removeMax(); // Throws if heap is empty
                            status[i] = IN_FLIGHT;
                            ts[i] = thread(loadThread, dir, next, q, status + i);
                            cout << "LOAD: Creating thread to load " << next << "\n";
                            inFlight++;
                        }
                        catch (...) { this_thread::sleep_for(chrono::milliseconds(100)); }
                    } else {
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }
                }
            }     
        }
    }
}

using namespace COS518;

int main(int argc, char** argv) {
  // Return an error if an improper number of arguments are specified
  if (argc != 5) {
      cerr << "Usage: client [Picture Directory] [Server URL] [Server Port] [Unique ID]\n";
      return 1;
  }

  // Initialize datastructures
  string dir(argv[1]);
  FileMgr *fm = new FileMgr(dir);
  ByteQueue *q = new ByteQueue();
  AckSet *s = new AckSet();
  ClientSocket *sock = new ClientSocket(argv[3], argv[4]);
  
  // Begin threads that are never supposed to crash (capture and load)
  thread load(loadMaster, dir, fm, q, 2, 2);
  thread capt(captureThread, dir, fm);
  
  // Begin threads that crash if the socket closes
  for (; ;) {
      // Reopen the socket if it has closed
      while (!sock->isOpen()) {
          delete sock;
          sock = new ClientSocket(argv[3], argv[4]);
          cerr << "Socket is recovering\n";
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      
      // Initialize the sender and acknowledgement threads
      thread send(sendThread, q, s, sock, stoi(string(argv[2])), 0);
      thread acks(ackThread, s, sock);
      
      // Both threads crash if the socket closes
      send.join();
      acks.join();
  }
  
  // Should never get here
  load.join();
  capt.join();
  return 1;
}
