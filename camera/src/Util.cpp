#include "Util.h"
#include <iostream>
#include <cstring>

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
}
