import os

STATIC_URL = "/static/"

DATABASE ='sqlite:///build/image_store.db'
#can only have one thing right now
STATIC_DIR = {
	
	"path": os.getcwd() + "/build/"

}