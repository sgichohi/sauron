#include "ClientSocket.h"
#include "ThreadPool.h"
#include "Util.h"

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
using namespace COS518;

/*********************************************************************/
/* RECEIVE SENDABLEMATS IN A SIMPLE, STRAIGHTFORWARD FASION          */
/*********************************************************************/

int main(int argc, char **argv) {
    // Validate command line arguments
    if (argc < 3) {
        cerr << "Usage: server [camera address] [camera port]\n";
        cerr << "Options: [-b OR --benchmark] [-v OR --verbose] [-m OR --max INT] [-s OR --skip INT]\n";
        return 1;
    }

    // Handle options
    long max = 0;
    bool verbose = false;

    for (int i = 3; i < argc; i++) {
        string str(argv[i]);

        if (str == "-v" || str == "--verbose") verbose = true;
    }
    
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
        
        // Get the camera's id number
        char id;
        s->recv(&id, 1);
        if (verbose) cerr << "id: " << id << "\n";
        
        // Create a directory if it doesn't already exist
        stringstream sss;
        sss  << id;
        mkdir(sss.str().c_str(), 0755);
        
        // Count sent sendables
        int sent = 0;

        // Receive paths
        while(s->isOpen()) {
            // Declare variables
            char c = 0;
            string str = "";
            
            // Receive            
            try {
                s->recv(&c, 1);
                
                while (c != 0) {
                    str += c;
                    s->recv(&c, 1);
                }
            } catch (...) { break; }
            
            cout << str << "\n";
            
            sent++;
            
            // Exit if the maximum number of sendables has been hit
            if (max != 0 && sent == max) {
                s->close();
                exit(0);
            }
        }
    }
}
