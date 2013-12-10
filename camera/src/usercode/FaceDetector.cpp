#include "FaceDetector.h"
#include "SendableMat.h"
#include "../UserInterface.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
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
    return new SendableMat();
  }
  
  Transformer *UserFactory::getNewTransformer() {
    return new FaceDetector();
  }

  
  /********************************/
  /* FACE DETECTOR IMPLEMENTATION */
  /********************************/

  FaceDetector::FaceDetector() {
    String face_cascade_name = "../data/haarcascade_frontalface_alt.xml";
    face_cascade.load(face_cascade_name);
  }

  FaceDetector::~FaceDetector() {}
    
  // Initialize the iterator
  void FaceDetector::begin(Mat pic, long timestamp) {
    //fprintf(stderr, "entering begin.\n");
    // This code adapted from the tutorial at:
    // http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html

    curs.clear();

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
      SendableMat *sendable = new SendableMat();
      sendable->initialize(face_pic, timestamp, timestamp);
      curs.push_back(sendable);
    }
  }
    
  // Check whether the iterator is finished
  bool FaceDetector::finished() {
    return 0 == curs.size();
  }
  
  // Move the iterator to the next value
  void FaceDetector::next() {
    curs.pop_back();
  }

  Sendable *FaceDetector::current() {
    if (curs.size() > 0) {
      return curs.back();
    } else {
      return NULL;
    }
  }
}
