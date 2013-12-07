#include "ClientSocket.h"
#include "FaceDetector.h"

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

#include <opencv2/opencv.hpp>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>

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
        sss  << id;
        mkdir(sss.str().c_str(), 0755);
        
        
        while(s->isOpen()) {
            long ts, len;
            
            try {
                ts = s->recv();
                len = s->recv();
            } catch (...) { continue; }
            
            char b[len];
            try { s->recv(b, len); } catch (...) { continue; }
            
            SendableMats *sendable = new SendableMats();
            sendable->initialize(b);

            cerr << "received: " << ts << "-" << sendable->getScore() << "\n" << len << "bytes\n";

            vector<Mat> pics = sendable->getPics();

            delete sendable;
            
            for (size_t i = 0; i < pics.size(); i++) {
              stringstream ss;
              ss <<  id << "/" << ts << "-" << i << ".jpg";
              string filename = ss.str();
              imwrite(filename, pics[i]);
            }

            try {
           	    s->send(ts);
           	} catch (...) { continue; }
        }
    }
}
