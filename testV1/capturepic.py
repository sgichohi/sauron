import cv2
from utils import ensure_dir
import time
from models import CameraFrame

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
    

def grabFrame(conn):
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


