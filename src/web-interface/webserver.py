from flask import Flask
from flask import request
from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop
from tornado import web
from mapper import maps
from db import session
from settings import STATIC_URL, STATIC_DIR, DATABASE
"""
Here we have a web application that simply handles routing 
between transformations and the getters exposed via HTTP

"""
app = Flask(__name__)


@app.route("/")
def hello():
    return "To run this web server on \
   		  a transform go to /do/<transform class name>/<function>/"

@app.route("/do/<transform>/",  methods=['GET', 'POST'])
def apply_transform(transform):
	"""
	This function applies a transform from maps, as assigned in mapper.py
	"""
	print 'Transform %s' % transform
	
	if transform in maps:
		return maps[transform](request=request, session=session)
	else:
		return "Either the transform or the function does not exist"


def runWebserver(port):
	wsgi_app = WSGIContainer(app)
	
	application = web.Application([
        (r"" + STATIC_URL + "(.*)", web.StaticFileHandler, STATIC_DIR),
        (r".*", web.FallbackHandler, dict(fallback=wsgi_app)),
    ])
   	http_server = HTTPServer(application)
	http_server.listen(port)
	IOLoop.instance().start()



