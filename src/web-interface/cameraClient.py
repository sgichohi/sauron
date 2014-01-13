import socket
import struct
import cv2

class CameraClient:
	s = socket.socket()
	cam_id = -1
	buf_size = 64
	
	def __init__(self, host, port=5000):
		#self.s = socket.socket()
		self.s.connect((host, port))
	#receive the camera's unique ID
		#self.long_size = self.leUnpack(self.s.recv(1))
		self.cam_id = self.leUnpack(self.s.recv(1))
		print "Camera: " + str(self.cam_id)
	def Id(self):
		return self.cam_id
	def lastLamport(self):
		return self.last_lamport
	def getDir(self):
		"""An iterator used as follows  
		for dir in cam.getDir():
			print dir
		 """
		return self.linesplit()
		
		
		#return str(s)
	def leUnpack(self, byte):
    		""" Converts byte string to integer. Use Little-endian byte order."""
		return sum([
                ord(b) << (8 * i) for i, b in enumerate(byte)
                    ])
	
	def linesplit(self):
    # untested
	    buff = self.s.recv(self.buf_size)
	    done = False
	    while not done:
	        if "\0" in buff:
	            (line, buff) = buff.split('\0', 1)
	            yield line
	        else:
	            more = self.s.recv(self.buf_size)
	            if not more:
	                done = True
	            else:
	                buff = buff+more
	    if buff:
	        yield buff
	def __del__ (self):
		self.s.close()







































