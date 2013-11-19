#include "AbstractSocket.h"
#include <cerrno>
#include <cstring>
#include "Util.h"

using namespace std;

namespace COS518 {

	bool AbstractSocket::isOpen() { return socketOpen;}
	
	int AbstractSocket::number() { return sock; }
	
	void AbstractSocket::handleError() {
	    close();
	}
	
	void AbstractSocket::send(char *msg, int len) throw(int) {
	    if (!socketOpen) throw -1;
	    
	    int sent = 0;
	    while (sent < len) {
	        int temp_sent = ::send(sock, msg + sent, len - sent, 0);
	        
	        // Handle the error case
	        if (temp_sent == -1) {
	            cerr << "Send error: " << strerror(errno) << "\n";
	            handleError();
	            throw -1;
	        }
	        
	        sent += temp_sent;
	    }
	}
	
	void AbstractSocket::recv(char *buf, int len) throw(int) {
	    if (!socketOpen)  throw -1;

	    int rcvd = 0;
	    while (rcvd < len) {
	        int temp_rcvd = ::recv(sock, buf + rcvd, len - rcvd, 0);
	        
	        // Handle the error case
	        if (temp_rcvd <= 0) {
	            handleError();	            
	            if (temp_rcvd < 0) cerr << "Recv error: " << strerror(errno) << "\n";
	            
	            throw -1;
	        }
	        
	        rcvd += temp_rcvd;
	    }
	}
	
	long AbstractSocket::recv() throw(int) {
	    char buf[sizeof(long)];
	    recv(buf, sizeof(long));
	    return bytes_to_long(buf);
	}
	
	void AbstractSocket::send(long l) throw(int) {
	    char *b = long_to_bytes(l);
	    try { send(b, sizeof(long)); } catch (...) { delete b; throw -1; }
	}
	
	void AbstractSocket::close() {
	    if (socketOpen) ::close(sock);
	    socketOpen = false;
	}
}     
