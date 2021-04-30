import ctypes
import numpy as np
import mmap
import struct

class HardwareFilterProcessing(object):
  def __init__(self):
    self.lib = ctypes.cdll.LoadLibrary('bin/imageProcessingDriver.so')
    result = self.lib.init()
    self.wholePackage = np.ones((480,752), dtype=np.uint64)
    
    self.f2 = open("/dev/mem", "r+b")
    self.switchMem = mmap.mmap(self.f2.fileno(), 1000, offset=0x44a20000)
    self.switchMem.seek(64)
    self.switchMem.write(struct.pack('l', 0))
    self.switchMem.seek(0)
    self.switchMem.write(struct.pack('l', 2))
    self.switchMem.close()
    self.f2.close()
  
  def getSegmentsWhole(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))
    return self.wholePackage

  def getSegments(self):
    result = self.lib.getFrame(ctypes.c_void_p(self.wholePackage.ctypes.data))

    dataIndex = 0
    endOfSegments = False

    segmentsN = []
    segmentsNW = []
    segmentsW = []
    segmentsSW = []

    # Holds a single segment to later be appended to the main segment list
    tempSegmentsN = []
    tempSegmentsNW = []
    tempSegmentsW = []
    tempSegmentsSW = []

    for i in range(len(self.wholePackage)):
      for j in range(len(self.wholePackage[0])):
        # First pixel is always 0
        if i == 0 and j == 0:
          continue

        pixelData = self.wholePackage[i,j]

        pixelDataN = np.right_shift(np.bitwise_and(pixelData,  np.uint64(int('0xFFFF000000000000', 16))), np.uint64(48))
        pixelDataNW = np.right_shift(np.bitwise_and(pixelData, np.uint64(int('0x0000FFFF00000000', 16))), np.uint64(32))
        pixelDataW = np.right_shift(np.bitwise_and(pixelData,  np.uint64(int('0x00000000FFFF0000', 16))), np.uint64(16))
        pixelDataSW = np.bitwise_and(pixelData, np.uint64(int('0x000000000000FFFF', 16)))

        # If all pixelData is 0, there are no more segments
        if pixelDataN == pixelDataNW == pixelDataW == pixelDataSW == 0:
          endOfSegments = True
          print("Complete?: {}, {}".format(i, j))
          break
        
        # Append pixel data to the segment list
        tempSegmentsN.append(self._getSegmentData(pixelDataN, dataIndex))
        tempSegmentsNW.append(self._getSegmentData(pixelDataNW, dataIndex))
        tempSegmentsW.append(self._getSegmentData(pixelDataW, dataIndex))
        tempSegmentsSW.append(self._getSegmentData(pixelDataSW, dataIndex))

        # Increment data index
        dataIndex += 1

        # If received theta, save temps and clear
        if (dataIndex == 5):
          # Append the rows to the master list if non-zero
          if self._isValidTempSegments(segmentsN, tempSegmentsN):
            segmentsN.append(tempSegmentsN)
          if self._isValidTempSegments(segmentsNW, tempSegmentsNW):
            segmentsNW.append(tempSegmentsNW)
          if self._isValidTempSegments(segmentsW, tempSegmentsW):
            segmentsW.append(tempSegmentsW)
          if self._isValidTempSegments(segmentsSW, tempSegmentsSW):
            segmentsSW.append(tempSegmentsSW)
          # Clear temp lists
          tempSegmentsN = []
          tempSegmentsNW = []
          tempSegmentsW = []
          tempSegmentsSW = []
          # Reset Data Index
          dataIndex = 0
      # Check if there were no valid segment data points
      if endOfSegments:
        break
    
    return segmentsN + segmentsNW + segmentsW + segmentsSW

  def _getSegmentData(self, pixelData, dataIndex):
    import fixedPointUtil as fpu
    parsedData = pixelData
    # Data is actually theta which is s10.7
    if (dataIndex == 4):
      parsedData = fpu.QX_XtoFloat(parsedData, 10, 7)
    
    return parsedData

  def _isValidTempSegments(self, segments, tempSegments):
    return len(tempSegments) > 0 and tempSegments[0] != 0
