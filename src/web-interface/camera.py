from db import session
from webserver import runWebserver
from multiprocessing import Process, Pipe
from capturepic import saveImage, grabFrame
import os
import sys

if __name__ == "__main__":
	if len(sys.argv) < 3:
		print "Run as python camera.py recipientport webserverport"

	rec_port = sys.argv[1]
	webport = sys.argv[2]
	#start the web server in its distinct directory.
	#Message passing between frame IO Loop and storing image
	save_image, grab_frame = Pipe(False)
	#start the webserver
	webserver = Process(target=runWebserver, args=(webport,))
	#grab frames in one prcess
	camera_listener = Process(target=grabFrame, args=(grab_frame,rec_port,))
	#store frames in another process
	parent_dir = os.getcwd() + "/" + "build/output/"
	frame_store = Process(target=saveImage, args=(save_image, parent_dir, session))
	camera_listener.start()
	frame_store.start()
	webserver.start()
	webserver.join()
	camera_listener.join()
	frame_store.join()
    