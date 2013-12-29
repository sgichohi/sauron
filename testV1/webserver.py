from flask import Flask
from flask.ext.sqlalchemy import SQLAlchemy
import cv2
app = Flask(__name__)
db = SQLAlchemy(app)

@app.route("/")
def hello():
    return "Hello World!"
@app.route("/faces")
def faces():
	
	return "face"

