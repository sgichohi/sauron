"""
This is a simple example of a "receiver" function. It gets
all the request arguments from a web call.

By default I pass the database pointer and a request object.

"""
import simplejson as json
from models import CameraFrame
import os
import settings
import cv2

def identity(*args, **kwargs):
	"""
	Basically shows all images
	"""

	#the database session
	#tips on how to use the session here: http://docs.sqlalchemy.org/en/latest/orm/session.html
	session = kwargs["session"]
	#request params from http
	request = kwargs["request"]
	print request

	#for each image we have so far, get the location
	image_locations = [x.location for x in session.query(CameraFrame).all()][:10]
	#read the file into the buffer

	#decode
	#image_decoded = cv2.imdecode()
	print image_locations
	#convert to sth the file server can give you
	image_static = ["/static/" + direc for direc in image_locations]
	return json.dumps(image_static)