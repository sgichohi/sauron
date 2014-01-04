

from flask import Flask
from flask.ext.sqlalchemy import SQLAlchemy
from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop
from tornado import web
import os
"""
Here we have a web application that simply handles routing 
between transformations and the getters exposed via HTTP

"""
app = Flask(__name__)
db = SQLAlchemy(app)



@app.route("/")
def hello():
    return "Hello World!"


@app.route("/faces")
def faces():
	
	return "face"

def runWebserver():
	wsgi_app = WSGIContainer(app)
	
	application = web.Application([
        (r"/static/(.*)", web.StaticFileHandler, {"path": os.getcwd() + "/build/"}),
        (r".*", web.FallbackHandler, dict(fallback=wsgi_app)),
    ])
   	http_server = HTTPServer(application)
	http_server.listen(5000)
	IOLoop.instance().start()



