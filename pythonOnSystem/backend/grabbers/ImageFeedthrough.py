import ctypes
import numpy as np

class ImageFeedthrough(object):
  def __init__(self):
    self.lib = ctypes.cdll.LoadLibrary('bin/imageFeedthroughDriver.so')
    result = self.lib.init()
    self.frame= np.ones((480,752,8), dtype=np.uint8)
    
  def getStereoRGB(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.frame.ctypes.data))
    return self.frame[:,:,0:3],self.frame[:,:,4:7]

  def getStereoGray (self):
    result = self.lib.getFrame(ctypes.c_void_p(self.frame.ctypes.data))
    return self.frame[:,:,3],self.frame[:,:,7]
    
  def getStereoAll (self):
    result = self.lib.getFrame(ctypes.c_void_p(self.frame.ctypes.data))
    return self.frame[:,:,0:4],self.frame[:,:,4:8]
 
  def __del__(self):
    result = self.lib.destroy
