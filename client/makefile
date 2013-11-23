#------------------------------------------------
# Macros
#------------------------------------------------
CC = g++
CCFLAGS = -Wall -std=c++11 -pthread -O3

#------------------------------------------------
# Build client .o files
#------------------------------------------------
UserDefined.o : UserDefined.h UserDefined.cpp
	$(CC) $(CCFLAGS) -c UserDefined.cpp
	
Entry.o : Entry.h UserDefined.h Entry.cpp
	$(CC) $(CCFLAGS) -c Entry.cpp
	
FileMgr.o : FileMgr.h FileMgr.cpp
	$(CC) $(CCFLAGS) -c FileMgr.cpp

	
AbstractSocket.o : Util.h AbstractSocket.h AbstractSocket.cpp
	$(CC) $(CCFLAGS) -c AbstractSocket.cpp
	
ClientSocket.o : AbstractSocket.h ClientSocket.h ClientSocket.cpp
	$(CC) $(CCFLAGS) -c ClientSocket.cpp

Util.o : Util.h Util.cpp
	$(CC) $(CCFLAGS) -c Util.cpp

client.o : ClientSocket.h Entry.h FileMgr.h UserDefined.h Util.h client.cpp
	$(CC) $(CCFLAGS) -c client.cpp

#------------------------------------------------
# Build server .o files
#------------------------------------------------

ServerSocket.o : AbstractSocket.h  ServerSocket.h ServerSocket.cpp
	$(CC) $(CCFLAGS) -c ServerSocket.cpp

server.o : ServerSocket.h server.cpp
	$(CC) $(CCFLAGS) -c server.cpp

#------------------------------------------------
# Build binaries
#------------------------------------------------
CLIENT_DEPS = FileMgr.o Entry.o ClientSocket.o AbstractSocket.o Util.o UserDefined.o client.o
client : $(CLIENT_DEPS)
	$(CC) $(CCFLAGS) $(CLIENT_DEPS) -o client

server : ServerSocket.o AbstractSocket.o Util.o server.o
	$(CC) $(CCFLAGS) ServerSocket.o Util.o AbstractSocket.o server.o -o server

all : client server

clean:
	rm -f *.o client server
