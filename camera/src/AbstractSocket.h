#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

namespace COS518 {
	class AbstractSocket {
		protected:
		
		int sock;
		bool socketOpen;
		void handleError();
		
		public:
	    bool isOpen();
	    void send(char* msg, int len) throw(int);
	    void recv(char* buf, int len) throw(int);
	    void send(long) throw(int);
	    long recv() throw(int);
	    int  number();
	    void close();
	};
}

#endif    
