import cv2
import remi.gui as gui
from remi import start, App
import logging
import numpy as np
import struct
import ctypes

from backend.grabbers.ImageFeedthrough import ImageFeedthrough

global camera
camera = ImageFeedthrough()

time.sleep(1)
frameLeft,frameRight = camera.getStereoRGB()
cv2.imwrite("left.jpg", frameLeft)
cv2.imwrite("right.jpg", frameRight)