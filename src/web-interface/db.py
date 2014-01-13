from models import initialise_db
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from utils import ensure_dir
from settings import DATABASE as direc
import os


def initialise():
	print ("creating a database")
	#direc ='sqlite:///image_store.db'

	initialise_db(direc)


def database_ish():
	ensure_dir(os.getcwd()  + "/build/")
	initialise()
	image_store = create_engine(direc)
	Session = sessionmaker(bind=image_store)
	session = Session()
	return session

session = database_ish()

