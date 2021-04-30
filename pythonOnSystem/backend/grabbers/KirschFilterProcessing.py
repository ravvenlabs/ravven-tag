import ctypes
import numpy as np
import mmap
import struct

class KirschFilterProcessing(object):
  def __init__(self):
    self.lib = ctypes.cdll.LoadLibrary('bin/imageProcessingDriver.so')
    result = self.lib.init()
    self.wholePackage = np.ones((480,752,8), dtype=np.uint8)
    
    self.f2 = open("/dev/mem", "r+b")
    self.switchMem = mmap.mmap(self.f2.fileno(), 1000, offset=0x44a20000)
    self.switchMem.seek(64) 
    self.switchMem.write(struct.pack('l', 0))
    self.switchMem.seek(0) 
    self.switchMem.write(struct.pack('l', 2))
    self.switchMem.close()
    self.f2.close()
    
  def getBinaryImages(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    mag_bin = np.left_shift(np.bitwise_and(self.wholePackage[:,:,0], int('0x10', 16)), 3)
    kirsch_n = np.left_shift(np.bitwise_and(self.wholePackage[:,:,0], int('0x08', 16)), 4)
    kirsch_nw = np.left_shift(np.bitwise_and(self.wholePackage[:,:,0], int('0x04', 16)), 5)
    kirsch_w = np.left_shift(np.bitwise_and(self.wholePackage[:,:,0], int('0x02', 16)), 6)
    kirsch_sw = np.left_shift(np.bitwise_and(self.wholePackage[:,:,0], int('0x01', 16)), 7)
    return mag_bin, kirsch_n, kirsch_nw, kirsch_w, kirsch_sw
    
  def getMagTheta(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    mag = self.wholePackage[:,:,2]
    theta = self.wholePackage[:,:,3]
    return mag, theta

  def getIxIy(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    ix = self.wholePackage[:,:,4]
    iy = self.wholePackage[:,:,5]
    return ix, iy

  def getGaussianSmoothed(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    gaussian_smoothed = self.wholePackage[:,:,6]
    return gaussian_smoothed

  def getGray(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    return self.wholePackage[:,:,7]
    
  def __del__(self):
    result = self.lib.destroy