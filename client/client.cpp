#include "FileMgr.h"
#include "ClientSocket.h"
#include "UserDefined.h"
#include <thread>
#include <chrono>
#include <istream>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace UserDefined;

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
	    fread(buf, 1, 100, fp)
	    fclose(fp);
	    
	    // Write timestamp file
	    lamport = stol(buf);
	    ss << (lamport + addend);
	    fp = fopen(timefile.c_str(), "wb");
	    fwrite(ss.str().c_str(), 1, ss.str().length(), fp);
	    fclose(fp);
	    
	    return lamport;
	}
    
    void captureThread(string directory, FileMgr *fm, string timefile) {
        long lamport = extendLamport(timefile, 1000);
        long limit   = lamport + 1000;
        Transformer trfm;
        
        for (; ; lamport++) {
            // Capture a new picture
            char *pic = (char *)"hello!"; // Replace
            int   len = 7; // Replace
            long  ts  = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
           
            // Retrieve sendables to place in the FileMgr
            for (trfm.begin(pic, len, ts); !trfm.finished(); trfm.next()) {
                // Update the lamport time if necessary
		        if (lamport >= limit) {
		            lamport = extendLamport(timefile, 1000);
		            limit   = lamport + 1000;
		        } 
                
                // Create a filename for the sendable
                stringstream ss;
                ss << lamport << "-" << trfm.current()->score() << ".jpg";
                string filename = ss.str();
                string path = directory + "/" + filename;
                
                // Save the sendable to a file with a unique name
                int outlen = 0;
                char *towrite = trfm.current()->serialize(&outlen);
                FILE *file = fopen(path.c_str(), "wb");
                fwrite(towrite, 1, outlen, file);
                fclose(file);
                delete towrite;
                
                // Insert the sendable object into the FileMgr
                fm->insert(lamport, trfm.current()->score(), filename);
	            cerr << "CAPTURE: generated file " << filename << "\n"; 
	        } 
		}
    }
    
    /*******************************************************************************/
    /* ACKNOWLEDGEMENT THREAD                                                      */
    /*-----------------------------------------------------------------------------*/
    /* Wait for acknowledgements from the server.  When received, delete the file  */
    /* that has been acknowledged and wait for another acknowledgement.  Crashes   */
    /* if the provided socket fails for any reason.                                */
    /*******************************************************************************/
    void ackThread(FileMgr *fm, ClientSocket *sock) {
        for (; ;) {
            try {
                long l = sock->recv();
                cerr << "ACK: Acknowledgement received for timestamp " << l << "\n";
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
    void sendThread(FileMgr *fm, ClientSocket *sock, int id, int maxUnacked = 0) {        
        long ts;
        char *buf;
        int len;
        string filename;
        
        // Send the client's ID number to the server.  Crash the thread on failure.
        try { sock->send((long)id); } catch(...) { return; }
        
        // Loop forever sending items from the FileManager's queue
        for (; ;) {
            // If the maximum number of items is currently outstanding, sleep
            while (maxUnacked > 0 && fm->ackSize() >= maxUnacked) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            
            // Attempt to take a picture from the FileManager
            while (true) {
                try { buf = fm->nextToAcks(&ts, &len, filename); }
                
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
                fm->enqueue(ts, buf, len, filename);
                return;
            }
            
            // On success, add an entry to the AckSet and deallocate the buffer memory
            cerr << "SEND: Picture " << ts << " successfully sent\n";
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
    
    void loadThread(FileMgr *fm, int *status) {
        try { fm->nextToQueue(); } catch (...) { }
        *status = FINISHED;
    }

    // Manages a pool of loadThreads
    void loadMaster(string dir, FileMgr *fm, int maxInFlight, int maxInQ) {
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
                    if (inFlight < maxInFlight &&
                        fm->queueSize() + inFlight < maxInQ &&
                        !fm->heapIsEmpty())
                    {
                            status[i] = IN_FLIGHT;
                            ts[i] = thread(loadThread, fm, status + i);
                            inFlight++;
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
    if (argc != 4) {
        cerr << "Usage: client [Unique ID] [Server URL] [Server Port]\n";
        return 1;
    }
  
    // Initialize filestructure
    string dir = "output";
    string cfg = "config";
    string timefile = cfg + "/time.txt";
  
    mkdir(dir.c_str(), 0755);
    mkdir(cfg.c_str(), 0755);
    
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
    ClientSocket *sock = new ClientSocket(argv[2], argv[3]);
  
    // Begin threads that are never supposed to crash (capture and load)
    thread load(loadMaster, dir, fm, 2, 2);
    thread capt(captureThread, dir, fm, timefile);
    
  
    // Begin threads that crash if the socket closes
    for (; ;) {
        // Reopen the socket if it has closed
        while (!sock->isOpen()) {
            delete sock;
            sock = new ClientSocket(argv[2], argv[3]);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
      
        // Initialize the sender and acknowledgement threads
        thread send(sendThread, fm, sock, stoi(string(argv[1])), 0);
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
