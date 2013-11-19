#include "ServerSocket.h"
#include <thread>
#include <chrono>
#include <istream>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sstream>

using namespace std;
using namespace COS518;

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: server [Port to listen on]\n";
        return 1;
    }
    
    Acceptor a(argv[1]);
    
    for (; ;) {
        cerr << "Accepting connections...\n";
        ServerSocket s(-1);
        try {
            s = a.accept();
            cerr << "Connection accepted on socket " << s.number() << "\n";
        } catch (...) {
            cerr << "Accept failed, rinsing and repeating\n";
            continue;
        }

        long id = s.recv();
        cerr << "id: " << id << "\n";
        
        while(s.isOpen()) {
            long ts, len;
            
            try {
                ts = s.recv();
                len = s.recv();
            } catch (...) { continue; }
            
            char b[len];
            try { s.recv(b, len); } catch (...) { continue; }
            
            stringstream ss;
            ss <<  id << "/" << ts << ".jpg";
            
            string filename = ss.str();
            
            cerr << "received: " << filename << "\n" << len << "bytes\n";
            
            FILE *file = fopen(filename.c_str(), "wb");
            fwrite(b, 1, len, file);
            fclose(file);
            
            try {
           	    s.send(ts);
           	} catch (...) { continue; }
        }
    }
}
