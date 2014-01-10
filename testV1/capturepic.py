import cv2
from utils import ensure_dir
import time
from models import CameraFrame
from cameraClient import CameraClient

def saveImage(conn, parent_dir, session):
    ensure_dir(parent_dir)
    count = 0L
    while True:
        timestamp = int(time.time())
        fileLocation = parent_dir + 'raw_frame_' + str(count) + "_" + str(timestamp) +  '.jpg'
        frame = conn.recv()
        cv2.imwrite(fileLocation, frame)
        fr = CameraFrame(location=fileLocation, lamport_time=count)
        session.add(fr)
        if count % 10 == 0:
            session.commit()
        count += 1
    session.commit()
    conn.close()

def newgrabFrame(conn, port):
    cam = CameraClient('127.0.0.1', port)
    while True:

        im = cam.getResults()
        conn.send(im)
        cv2.imshow('frame',im)
        if cv2.waitKey(1) & 0xFF == ord('q'):
               break
    conn.close()
    cv2.destroyAllWindows()




def grabFrame(conn, port):
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


