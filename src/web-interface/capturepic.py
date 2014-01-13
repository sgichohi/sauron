
from utils import ensure_dir
import cv2
from models import CameraFrame
from cameraClient import CameraClient

def saveImage(conn, parent_dir, session):
    ensure_dir(parent_dir)
    count = 0L
    while True:
        
        fileLocation = conn.recv()
        fr = CameraFrame(location=fileLocation, lamport_time=count)
        session.add(fr)
        if count % 10 == 0:
            session.commit()
        count += 1
    session.commit()
    conn.close()

def grabFrame(conn, port):
    cam = CameraClient('127.0.0.1', int(port))
    while True:
        #im = cam.getDir()
        for im in cam.getDir():
            conn.send(im)




def ngrabFrame(conn, port):
    """Grabs a frame from the network"""
    cap = cv2.VideoCapture(0)
    
    while(cap.isOpened()):
        ret, frame = cap.read()
        if ret==True:
            conn.send(frame)
            cv2.imshow('frame',frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
               break            
        else:
            break
    cap.release()
    conn.close()
    cv2.destroyAllWindows()


