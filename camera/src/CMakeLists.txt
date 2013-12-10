cmake_minimum_required(VERSION 2.8)
project(Camera)

find_package( OpenCV REQUIRED )

# Add the compiler flags
SET( COMPILE_FLAGS "-Wall -std=gnu++11 -O3 -pthread -D_GLIBCXX_USE_NANOSLEEP" )
SET( CMAKE_CXX_FLAGS "${COMPILE_FLAGS}" )

# Create the executables
add_executable( camera_face_detector camera.cpp FileMgr.cpp Entry.cpp
  ThreadPool.cpp ServerSocket.cpp AbstractSocket.cpp Util.cpp
  usercode/FaceDetector.cpp usercode/SendableMat.cpp )

add_executable( camera_do_nothing camera.cpp FileMgr.cpp Entry.cpp
  ThreadPool.cpp ServerSocket.cpp AbstractSocket.cpp Util.cpp
  usercode/DoNothing.cpp usercode/SendableMat.cpp )

add_executable( camera_difference_rater camera.cpp FileMgr.cpp Entry.cpp
   ThreadPool.cpp ServerSocket.cpp AbstractSocket.cpp Util.cpp
    usercode/DifferenceRater.cpp usercode/SendableMat.cpp )


add_executable( recipient recipient.cpp ClientSocket.cpp
  AbstractSocket.cpp Util.cpp usercode/SendableMat.cpp )

# Tell CMake to link with opencv
target_link_libraries( camera_do_nothing ${OpenCV_LIBS} )
target_link_libraries( camera_difference_rater ${OpenCV_LIBS} )
target_link_libraries( camera_face_detector ${OpenCV_LIBS} )
target_link_libraries( recipient ${OpenCV_LIBS} )
