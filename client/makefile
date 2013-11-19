#------------------------------------------------
# Macros
#------------------------------------------------
CC = g++
CCFLAGS = -Wall -Werror -std=c++11 -pthread

#------------------------------------------------
# Build .o files
#------------------------------------------------
FileMgr.o : FileMgr.h FileMgr.cpp
	$(CC) $(CCFLAGS) -c FileMgr.cpp

ByteQueue.o : ByteQueue.h ByteQueue.cpp
	$(CC) $(CCFLAGS) -c ByteQueue.cpp

AckSet.o : AckSet.h AckSet.cpp
	$(CC) $(CCFLAGS) -c AckSet.cpp
	
AbstractSocket.o : Util.h AbstractSocket.h AbstractSocket.cpp
	$(CC) $(CCFLAGS) -c AbstractSocket.cpp
	
ClientSocket.o : AbstractSocket.h ClientSocket.h ClientSocket.cpp
	$(CC) $(CCFLAGS) -c ClientSocket.cpp

Util.o : Util.h Util.cpp
	$(CC) $(CCFLAGS) -c Util.cpp

client.o : AckSet.h ClientSocket.h ByteQueue.h FileMgr.h Util.h client.cpp
	$(CC) $(CCFLAGS) -c client.cpp


ServerSocket.o : AbstractSocket.h  ServerSocket.h ServerSocket.cpp
	$(CC) $(CCFLAGS) -c ServerSocket.cpp

server.o : ServerSocket.h server.cpp
	$(CC) $(CCFLAGS) -c server.cpp

#------------------------------------------------
# Build binary
#------------------------------------------------
client : FileMgr.o ByteQueue.o AckSet.o ClientSocket.o AbstractSocket.o Util.o client.o
	$(CC) $(CCFLAGS) FileMgr.o ByteQueue.o AckSet.o AbstractSocket.o ClientSocket.o Util.o client.o -o client

server : ServerSocket.o AbstractSocket.o Util.o server.o
	$(CC) $(CCFLAGS) ServerSocket.o Util.o AbstractSocket.o server.o -o server

all : client server

clean:
	rm -f *.o client server
