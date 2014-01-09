#include "ClientSocket.h"
#include <cerrno>
#include <cstring>
#include "Util.h"

using namespace std;

namespace COS518 {

	ClientSocket::ClientSocket(char * host, char* port) {
	    socketOpen = false;
	    
		// Declare structures
		int status;
		struct addrinfo hints;
		struct addrinfo *res;

		// Configure hints
		memset(&hints, 0, sizeof(hints));
		hints.ai_family   = AF_UNSPEC; // IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP

		if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
			cerr << "getaddrinfo failed: " << gai_strerror(status);
			return;
		}
	
		// Retrieve a socket and connect
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
			//cerr << "Connect error : " << strerror(errno) << "\n";
			::close(sock);
			freeaddrinfo(res);
			return;
		}
	
		// Complete
		freeaddrinfo(res);
		socketOpen = true;
	}
}     
