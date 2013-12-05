cmake_minimum_required(VERSION 2.8)
project(Camera)
find_package( OpenCV REQUIRED )

# Add the compiler flags
SET( COMPILE_FLAGS "-Wall -std=gnu++11 -pthread -O3 -D_GLIBCXX_USE_NANOSLEEP" )
SET( CMAKE_CXX_FLAGS "${COMPILE_FLAGS}" )

# Create the executables
add_executable( camera camera.cpp FileMgr.cpp Entry.cpp ThreadPool.cpp ServerSocket.cpp AbstractSocket.cpp Util.cpp DifferenceRater.cpp )
add_executable( recipient recipient.cpp ClientSocket.cpp AbstractSocket.cpp Util.cpp
DifferenceRater.cpp )

# Tell CMake to link with opencv
target_link_libraries( camera ${OpenCV_LIBS} )
target_link_libraries( recipient ${OpenCV_LIBS} )