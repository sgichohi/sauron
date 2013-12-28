from flask import Flask
from multiprocessing import Process
from models import initialise_database
from capturepic import _saveImage, _grabFrame

app = Flask(__name__)

def initialise():
	print ("creating a database")
	direc ='sqlite:///image_store.db'
	initialise_database(direc)

@app.route("/")
def hello():
    return "Hello World!"

if __name__ == "__main__":
	initialise()
	#start the web server in its distinct directory.
	webserver = Process(target=app.run, args=())
	camera_listener = Process(target=)
	webserver.start()
	webserver.join()
    