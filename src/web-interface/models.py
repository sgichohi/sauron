
from sqlalchemy import create_engine
from sqlalchemy import Column, Date, Integer, BigInteger, String
from sqlalchemy.ext.declarative import declarative_base
import datetime

Base = declarative_base()
 
########################################################################
class CameraFrame(Base):
    """ This is the frame"""
    __tablename__ = "cframe"
 
    id = Column(Integer, primary_key=True)
    location = Column(String) 
    date_created = Column(Date)
    lamport_time = Column(BigInteger)
 
    #----------------------------------------------------------------------
    def __init__(self, location, lamport_time):
        """"""
        self.location = location  
        self.lamport_time = lamport_time
        self.date_created = datetime.datetime.now()
 
########################################################################

def initialise_db(direc):

    
    engine = create_engine(direc, echo=True)
    # create tables
    Base.metadata.create_all(engine)
   
