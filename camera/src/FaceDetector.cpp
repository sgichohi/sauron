#include "FaceDetector.h"
#include "UserInterface.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <climits>
#include <iostream>

using namespace cv;
using namespace std;

namespace UserDefined {
  
  /*******************************/
  /* USER FACTORY IMPLEMENTATION */
  /*******************************/

  Sendable *UserFactory::getNewSendable() {
    return new SendableMats();
  }

  Transformer *UserFactory::getNewTransformer() {
    return new FaceDetector();
  }

  /*******************************/
  /* SENDABLE MAT IMPLEMENTATION */
  /*******************************/

  void SendableMats::initialize(vector<Mat> pics, long timestamp, long score) {
    this->score = score;
    this->timestamp = timestamp;

    // Note that this assignment does not copy the underlying array. Rather, it
    // just makes another reference to the same data. When the caller destructs
    // its local version of pic, this will be the only reference to the array.
    // tl;dr: this assignment is intantaneous and OpenCV manages memory for us.
    this->pics = pics;
  }
    
  // Deserializer constructor
  void SendableMats::initialize(char *b) {
    //fprintf(stderr, "entering initialize.\n");
    int inLength;
    memcpy(&inLength, b, sizeof(inLength));
    memcpy(&timestamp, b + sizeof(inLength), sizeof(timestamp));
    memcpy(&score, b + sizeof(inLength) + sizeof(timestamp), sizeof(score));

    pics = vector<Mat>();

    char *offset = b + sizeof(inLength) + sizeof(timestamp) + sizeof(score);
    char *end_offset = b + inLength;
    while (offset != end_offset) {
      size_t buf_len;
      memcpy(&buf_len, offset, sizeof(size_t));
      offset += sizeof(size_t);
      
      vector<uchar> buf;
      buf.reserve(buf_len);
      buf.assign(offset, offset + buf_len);
      offset += buf_len;

      pics.push_back(imdecode(buf, 1));
    }
    //fprintf(stderr, "leaving initialize.\n");
  }
    
  // Serializer
  char *SendableMats::serialize(int *outLength) {
    //fprintf(stderr, "entering serialize.\n");
    int bufs_len = 0;
    vector<vector<uchar> > bufs;
    for (vector<Mat>::iterator it = pics.begin(); it != pics.end(); it++) {
      vector<uchar> buf;
      imencode(".jpg", *it, buf);
      bufs_len += buf.size();
      bufs_len += sizeof(size_t);
      bufs.push_back(buf);
    }

    *outLength = sizeof(*outLength) + sizeof(timestamp) + sizeof(score) + bufs_len;
    char *b = new char[*outLength];
    memcpy(b, outLength, sizeof(*outLength));
    memcpy(b + sizeof(*outLength), &timestamp, sizeof(timestamp));
    memcpy(b + sizeof(*outLength) + sizeof(timestamp), &score, sizeof(score));

    char *offset = b + sizeof(*outLength) + sizeof(timestamp) + sizeof(score);
    for (vector<vector<uchar> >::iterator it = bufs.begin(); it != bufs.end(); it++) {
      size_t size = it->size();
      memcpy(offset, &size, sizeof(size_t));
      offset += sizeof(size_t);
      memcpy(offset, it->data(), it->size());
      offset += it->size();
    }
    
    //fprintf(stderr, "leaving serialize\n");
    return b;
  }
    
  const vector<Mat> SendableMats::getPics() { return pics; }

  SendableMats::SendableMats() {}
  SendableMats::~SendableMats() { 
    //fprintf(stderr, "entering sendablemats destructor.\n");
    for (vector<Mat>::iterator it = pics.begin(); it != pics.end(); it++) {
      it->release();
    }
    //fprintf(stderr, "leaving sendablemats destructor.\n");
  }

  /********************************/
  /* FACE DETECTOR IMPLEMENTATION */
  /********************************/

  FaceDetector::FaceDetector() {
    //fprintf(stderr, "entering FaceDetector constructor.\n");
    cur = NULL;
    full = false;

    String face_cascade_name = "../data/haarcascade_frontalface_alt.xml";
    face_cascade.load(face_cascade_name);
    //fprintf(stderr, "leaving FaceDetector constructor.\n");
  }

  // Destructor
  FaceDetector::~FaceDetector() {
    if (NULL != cur) delete cur;
  }
    
  // Initialize the iterator
  void FaceDetector::begin(Mat pic, long timestamp) {
    //fprintf(stderr, "entering begin.\n");
    // This code adapted from the tutorial at:
    // http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html

    const int scale = 4;
    const float float_scale = float(scale);
    vector<Rect> faces;
    Mat small_pic;
    resize(pic, small_pic, Size(), 1./float_scale, 1./float_scale, INTER_NEAREST);
    Mat pic_gray;

    cvtColor(small_pic, pic_gray, CV_BGR2GRAY);
    equalizeHist(pic_gray, pic_gray);

    face_cascade.detectMultiScale(pic_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30,30));

    vector<Mat> face_pics;
    for (vector<Rect>::iterator it = faces.begin(); it != faces.end(); it++) {
      Rect enlarged_rect = Rect(it->x*scale, it->y*scale,
                                it->width*scale, it->height*scale);
      Mat face_pic = pic(enlarged_rect).clone();
      face_pics.push_back(face_pic);
    }

    SendableMats *sendable = new SendableMats();
    sendable->initialize(face_pics, timestamp, face_pics.size());
    cur = sendable;
    
    full = true;

    //fprintf(stderr, "leaving begin.\n");
  }
    
  // Check whether the iterator is finished
  bool FaceDetector::finished() {
    return !full;
  }
  
  // Move the iterator to the next value
  void FaceDetector::next() {
    full = false;
  }

  Sendable *FaceDetector::current() {
    return cur;
  }
}
