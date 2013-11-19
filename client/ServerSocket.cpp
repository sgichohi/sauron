#include "ServerSocket.h"
#include <cerrno>
#include <cstring>
#include "Util.h"

using namespace std;

namespace COS518 {
    Acceptor::Acceptor(char * port) {
        socketOpen = false;
	    
		// Declare structures
		int status;
		struct addrinfo hints;
		struct addrinfo *res;

		// Configure hints
		memset(&hints, 0, sizeof(hints));
		hints.ai_family   = AF_UNSPEC; // IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP
		hints.ai_flags = AI_PASSIVE; // Server

		if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
			cerr << "getaddrinfo failed: " << gai_strerror(status);
			return;
		}
	
		// Retrieve a socket and bind
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
			cerr << "Bind error : " << strerror(errno) << "\n";
			::close(sock);
			freeaddrinfo(res);
			return;
		}
		
		// Listen
		listen(sock, 20);
	
		// Complete
		freeaddrinfo(res);
		socketOpen = true;
	}
	
	ServerSocket Acceptor::accept() throw(int) {
	    if (!socketOpen) throw -1;
	    
	    struct sockaddr x;
	    socklen_t sl = sizeof(x);
	    int newsock = ::accept(sock, &x, &sl);
	    
	    if (newsock == -1) {
	        cerr << "Accept error : " << strerror(errno) << "\n";
	        throw -1;
	    }

	    return ServerSocket(newsock);    
    }

	void Acceptor::close() {
	    if (socketOpen) ::close(sock);
	    socketOpen = false;
	}

	ServerSocket::ServerSocket(int newsock) {
	    socketOpen = true;
	    sock = newsock;
	}
}     
