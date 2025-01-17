#include "Util.h"
#include <iostream>
#include <cstring>
#include <chrono>

using namespace std;

namespace COS518 {
    char *long_to_bytes(long l) {
        char *b = new char[sizeof(long)];
        memcpy(b, &l, sizeof(long));
        return b;
    }
    
    long  bytes_to_long(char* b) {
        long l;
	    memcpy(&l, b, sizeof(long));
	    return l;
	}

    long now() {
        return chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
    }
}
