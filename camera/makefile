#------------------------------------------------
# Macros
#------------------------------------------------
CC = g++
CCFLAGS = -Wall -std=gnu++11 -pthread -O3 -D_GLIBCXX_USE_NANOSLEEP

#------------------------------------------------
# Build camera .o files
#------------------------------------------------
ThreadPool.o : ThreadPool.h ThreadPool.cpp
	$(CC) $(CCFLAGS) -c ThreadPool.cpp
	
UserDefined.o : UserDefined.h UserDefined.cpp
	$(CC) $(CCFLAGS) -c UserDefined.cpp
	
Entry.o : Entry.h UserDefined.h Entry.cpp
	$(CC) $(CCFLAGS) -c Entry.cpp
	
FileMgr.o : FileMgr.h FileMgr.cpp
	$(CC) $(CCFLAGS) -c FileMgr.cpp
	
AbstractSocket.o : Util.h AbstractSocket.h AbstractSocket.cpp
	$(CC) $(CCFLAGS) -c AbstractSocket.cpp

ServerSocket.o : AbstractSocket.h  ServerSocket.h ServerSocket.cpp
	$(CC) $(CCFLAGS) -c ServerSocket.cpp

Util.o : Util.h Util.cpp
	$(CC) $(CCFLAGS) -c Util.cpp

camera.o : AbstractSocket.h ThreadPool.h ServerSocket.h Entry.h FileMgr.h UserDefined.h Util.h camera.cpp
	$(CC) $(CCFLAGS) -c camera.cpp

#------------------------------------------------
# Build recipient .o files
#------------------------------------------------

ClientSocket.o : AbstractSocket.h ClientSocket.h ClientSocket.cpp
	$(CC) $(CCFLAGS) -c ClientSocket.cpp

recipient.o : ClientSocket.h recipient.cpp
	$(CC) $(CCFLAGS) -c recipient.cpp

#------------------------------------------------
# Build binaries
#------------------------------------------------
CAMERA_DEPS = FileMgr.o Entry.o ThreadPool.o ServerSocket.o AbstractSocket.o Util.o UserDefined.o camera.o
camera : $(CAMERA_DEPS)
	$(CC) $(CCFLAGS) $(CAMERA_DEPS) -o camera

recipient : ClientSocket.o AbstractSocket.o Util.o recipient.o
	$(CC) $(CCFLAGS) ClientSocket.o Util.o AbstractSocket.o recipient.o -o recipient

all : camera recipient

clean:
	rm -rf *.o camera recipient output config
