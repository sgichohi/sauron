import cv2
import os
from utils import ensure_dir
import time
#print os.getcwd()
def _saveImage(conn, frame):
    ensure_dir(parent_dir)
    timestamp = int(time.time())
    parent_dir 
    fileLocation = parent_dir + "/" + 'raw_frame'  + str(timestamp) +  ' .jpg'

    cv2.imwrite(fileLocation, frame)
    



def _grabFrame(conn):
    #get image from default camera, can be trivially changed to multiple cameras
    
    
    cap = cv2.VideoCapture(0)
    #print cap.get(cv2.cv.CV_CAP_PROP_FPS)
    while(cap.isOpened()):
        ret, frame = cap.read()

        if ret==True:
            #timestamp = int(time.time())
            #fileLocation = parent_dir + "/" + 'raw_frame'  + str(timestamp) +  ' .jpg'
            #cv2.imshow('frame',frame)
            #if cv2.waitKey(1) & 0xFF == ord('q'):
            #    break
            
            
        else:
            break

    cap.release()

    cv2.destroyAllWindows()


#record_video(os.getcwd() + "/" + "output/")