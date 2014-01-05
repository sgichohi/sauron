from flask import Flask
from flask import request
from flask.ext.sqlalchemy import SQLAlchemy
from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop
from tornado import web
from mapper import maps
import os
"""
Here we have a web application that simply handles routing 
between transformations and the getters exposed via HTTP

"""
app = Flask(__name__)
db = SQLAlchemy(app)



@app.route("/")
def hello():
    return "To run this web server on \
   		  a transform go to /do/<transform class name>/<function>/"

@app.route("/do/<transform>/<function>/",  methods=['GET', 'POST'])
def apply_transform(transform, function):

	print 'Transform %s' % transform + " " + function
	
	if transform in maps:
		return getattr(maps[transform], function)(request)
	else:
		return "Either the transform or the function does not exist"


def runWebserver():
	wsgi_app = WSGIContainer(app)
	
	application = web.Application([
        (r"/static/(.*)", web.StaticFileHandler, {"path": os.getcwd() + "/build/"}),
        (r".*", web.FallbackHandler, dict(fallback=wsgi_app)),
    ])
   	http_server = HTTPServer(application)
	http_server.listen(5000)
	IOLoop.instance().start()



