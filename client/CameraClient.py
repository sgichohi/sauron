import socket
import struct
class CameraClient:
	s = socket.socket()
	cam_id = -1
	last_lamport = 0
	long_size = 8
	def __init__(self, host, port=5000):
		#self.s = socket.socket()
		self.s.connect((host, port))
	#receive the camera's unique ID
		self.long_size = int(self.s.recv(1))
		self.cam_id = self.leUnpack(self.s.recv(self.long_size))
	def Id(self):
		return self.cam_id
	def lastLamport(self):
		return self.last_lamport
	def getResults(self):
		lamport_time = self.s.recv(self.long_size)
		last_lamport = self.leUnpack(lamport_time)
		self.last_lamport = last_lamport
		sendable_len = self.leUnpack(self.s.recv(self.long_size))
		self.s.send(str(last_lamport))
		return self.s.recv(sendable_len)
	def leUnpack(self, byte):
		print len(byte)
    		""" Converts byte string to integer. Use Little-endian byte order."""
		return sum([
                ord(b) << (8 * i) for i, b in enumerate(byte)
                    ])
	def __del__ (self):
		self.s.close()







































