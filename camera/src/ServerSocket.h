#include "AbstractSocket.h"

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

namespace COS518 {
	class ServerSocket : public AbstractSocket {
		public:
		ServerSocket(int);
	};
	
    class Acceptor {
        private:
        int sock;
        bool socketOpen;
        
        public:
        Acceptor(char *);
        void close();
        ServerSocket *accept() throw(int);
    };
}

#endif    
