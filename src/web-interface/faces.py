"""
This is a more complicated example of a "receiver" function.
It gets all the request arguments from a web call. The request
should include "first" and "last" arguments, that specify the
range of frames to process.

For example /do/faces/?first=7&last=16 would process the 7th
through 16th (inclusive) frames captured by the camera, 10
frames in all.

By default I pass the database pointer and a request object.

"""

import simplejson as json
from models import CameraFrame
import os
import settings
import cv2
import itertools
from identity import convert_to_jpg
face_cascade = None

def faces(*args, **kwargs):
        """
        Detects and shows faces
        """

        session = kwargs["session"]
        request = kwargs["request"]

        first_index = int(request.args.get("first")) 
        last_index = int(request.args.get("last"))

        image_locations = [convert_to_jpg(x.location) for x in session.query(CameraFrame).all()][first_index:last_index+1]
        #print len(image_locations)

        global face_cascade
        if face_cascade == None:
                face_cascade = cv2.CascadeClassifier("data/haarcascade_frontalface_alt.xml")


        faces_list_of_lists = map(detect_faces, image_locations)
        faces = list(itertools.chain.from_iterable(faces_list_of_lists))

        return json.dumps(faces)


def detect_faces(image_location):
        scale = 4
        inverse_scale = 1/float(scale)

        image = cv2.imread(image_location)

        small_image = None
        small_image = cv2.resize(image, (0, 0), small_image, inverse_scale, inverse_scale, cv2.INTER_NEAREST)
        gray_image = cv2.cvtColor(small_image, cv2.COLOR_BGR2GRAY)
        equalized_image = cv2.equalizeHist(gray_image)

        global face_cascade
        rectangles = face_cascade.detectMultiScale(equalized_image, scaleFactor=1.1, minNeighbors=2)
        scaled_rectangles = [tuple([x*scale for x in rect]) for rect in rectangles]
        
        faces = []
        for x, y, w, h in scaled_rectangles:
                face = image[y:y+h, x:x+w].copy()
                faces.append(face)

        face_locations = [ image_location[:-4] + "_face_" + str(i) + ".jpg" for i in range(len(faces)) ]
        print "face_locations", face_locations

        for (face, face_location) in zip(faces, face_locations):
                cv2.imwrite(face_location, face)

        face_locations_static = ["/static/" + os.path.relpath(direc, settings.STATIC_DIR["path"]) for direc in face_locations]

        return face_locations_static
