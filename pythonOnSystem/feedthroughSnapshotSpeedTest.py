import cv2
import remi.gui as gui
from remi import start, App
import logging
import numpy as np
import struct
import ctypes
import datetime

from backend.grabbers.ImageFeedthrough import ImageFeedthrough
 
global camera
camera = ImageFeedthrough()

number_of_frames = 100
now = datetime.datetime.now()
for _ in range(number_of_frames):
    frameLeft,frameRight = camera.getStereoRGB()
    
total_time = datetime.datetime.now() - now
frames_per_second = number_of_frames / total_time.total_seconds()
        
print("FPS {}".format(frames_per_second))