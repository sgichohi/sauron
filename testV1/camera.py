from webserver import runWebserver
from multiprocessing import Process, Pipe
from models import initialise_db
from capturepic import saveImage, grabFrame
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from utils import ensure_dir
import os


direc ='sqlite:///build/image_store.db'

def initialise():
	print ("creating a database")
	#direc ='sqlite:///image_store.db'

	initialise_db(direc)
	




if __name__ == "__main__":
	ensure_dir(os.getcwd()  + "/build/")
	initialise()
	image_store = create_engine(direc)
	Session = sessionmaker(bind=image_store)
	session = Session()
	#start the web server in its distinct directory.
	#Message passing between frame IO Loop and storing image
	save_image, grab_frame = Pipe(False)
	#start the webserver
	webserver = Process(target=runWebserver, args=())
	#grab frames in one prcess
	camera_listener = Process(target=grabFrame, args=(grab_frame,))
	#store frames in another process
	parent_dir = os.getcwd() + "/" + "build/output/"
	frame_store = Process(target=saveImage, args=(save_image, parent_dir, session))
	camera_listener.start()
	frame_store.start()
	webserver.start()
	webserver.join()
	camera_listener.join()
	frame_store.join()
    