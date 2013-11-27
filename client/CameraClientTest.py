from CameraClient import CameraClient

cam = CameraClient('127.0.0.1')
print cam.Id()
while True:
	sth = cam.getResults()
	print "got sth", cam.lastLamport()