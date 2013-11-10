#include "FileMgr.h"
#include <future>
#include <unordered_set>
#include <chrono>

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

    // Send the picture file in the specified directory with the specified filename to
    // the server.  When acknowledged, delete the file and return.
    enum { NOT_IN_USE, IN_FLIGHT, FINISHED};
    void transmitThread(string directory, string fileName, int* status) {
        cout << "Transmitting a thread..." + directory + "/" + fileName + "\n";
        
        // Retrieve picture file
        
        // Send file in queue along with metadata
        
        // Wait for ack
        
        // Delete picture file

        *status = FINISHED;
    }
    
/***********************************************************************************/
/* CODE FROM HERE DOWN IS COMPLETE                                                 */
/***********************************************************************************/

    // A function that never terminates, popping the maximum off of the queue and
    // transmitting it in a separate thread.
    void sendThread(string dir, int maxInFlight, FileMgr *fileMgr) {
        // Initialize thread-tracking information
        thread ts[maxInFlight];
        int inFlight = 0;
        
        int status[maxInFlight];
        for (int i = 0; i < maxInFlight; i++) status[i] = NOT_IN_USE;
        
        // Begin the main part of the function
        for (; ;) {
            // If we aren't at the maximum, transmit pictures until we are
            if (inFlight < maxInFlight) {
            
                // Attempt to insert a new thread at each empty spot in the pool
                for (int i = 0; i < maxInFlight && inFlight < maxInFlight; i++) {
                
                    // Keep trying to fill the spot until the heap has more information
                    while (status[i] == NOT_IN_USE) {
                        try {
                            string next = fileMgr->removeMax(); // Throws if heap is empty
                            status[i] = IN_FLIGHT;
                            ts[i] = thread(transmitThread, dir, next, status + i);
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

int main() {
  string dir = "hi";
  FileMgr *fm = new FileMgr(dir);
  thread send(sendThread, dir, 2, fm);
  thread capt(captureThread, dir, fm);
  send.join();
  capt.join();
  return 0;
}
