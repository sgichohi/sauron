import  socket

class CameraClient:
	s = socket.socket()
	cam_id = -1
	last_lamport = 0
	def __init__(self, host, port=5000):
		#self.s = socket.socket()
		self.s.connect((host, port))
	#receive the camera's unique ID
		self.cam_id = self.leUnpack(self.s.recv(4))
	def Id(self):
		return self.cam_id
	def lastLamport(self):
		return self.last_lamport
	def getResults(self):
		lamport_time = self.s.recv(4)
		last_lamport = self.leUnpack(lamport_time)
		self.last_lamport = last_lamport
		sendable_len = self.leUnpack(self.s.recv(4))
		self.s.send(lamport_time)
		return self.s.recv(sendable_len)
	def leUnpack(self, byte):
    		""" Converts byte string to integer. Use Little-endian byte order."""
		return sum([
        	ord(b) << (8 * i) for i, b in enumerate(byte)
   		 ])
	def __del__ (self):
		self.s.close()




cam = CameraClient('127.0.0.1')
i = 0
print cam.Id()
while True:
	sth = cam.getResults()
	print "got sth", cam.lastLamport()

































