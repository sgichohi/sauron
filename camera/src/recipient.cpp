#include "ClientSocket.h"
#include "DifferenceRater.h"

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

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;
using namespace UserDefined;
using namespace COS518;

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "Usage: server [camera address] [camera port]\n";
        return 1;
    }
    
    
    for (; ;) {
        cerr << "Initiating connection...\n";
        ClientSocket *s = new ClientSocket(argv[1], argv[2]);
        
        while (!s->isOpen()) {
            delete s;
            s = new ClientSocket(argv[1], argv[2]);
	        this_thread::sleep_for(chrono::milliseconds(1000));
        }
        
        char long_len;
        s->recv(&long_len, 1);
        cerr << "long_len: " << int(long_len) << "\n";
        
        long id = s->recv();
        cerr << "id: " << id << "\n";
        
        stringstream sss;
        sss << id;
        mkdir(sss.str().c_str(), 0755);
        
        
        while(s->isOpen()) {
            long ts, len;
            
            try {
                ts = s->recv();
                len = s->recv();
            } catch (...) { continue; }
            
            char b[len];
            try { s->recv(b, len); } catch (...) { continue; }
            
            SendableMat *sendable = new SendableMat();
            sendable->initialize(b);

            Mat pic = sendable->getPic();

            delete sendable;
            
            stringstream ss;
            ss <<  id << "/" << ts << ".jpg";
            
            string filename = ss.str();
            
            cerr << "received: " << filename << "\n" << len << "bytes\n";
            
            imwrite(filename, pic);

            try {
           	    s->send(ts);
           	} catch (...) { continue; }
        }
    }
}
