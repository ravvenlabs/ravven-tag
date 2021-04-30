import cv2
import remi.gui as gui
from remi import start, App
import logging
import numpy as np
import struct
import ctypes

from backend import fixedPointUtil as fpu
from backend.registerMap import RegisterMap, Register

from backend.grabbers.ImageFeedthrough import ImageFeedthrough
from backend.grabbers.ImageProcessing import ImageProcessing
from backend.grabbers.KirschFilterProcessing import KirschFilterProcessing

global cameraFeedthrough
global cameraProcessing
global cameraKirsch
cameraFeedthrough = ImageFeedthrough()
cameraProcessing  = ImageProcessing()
cameraKirsch = KirschFilterProcessing()

class VideoDisplayWidget(gui.Image):
    def __init__(self,fps=5, **kwargs):
        super(VideoDisplayWidget, self).__init__("/%s/get_image_data" % id(self), **kwargs)
        self.mode = 0
        self.fps = fps
        self.headers = {'Content-type': 'image/jpeg'}
        javascript_code = gui.Tag()
        javascript_code.type = 'script'
        javascript_code.attributes['type'] = 'text/javascript'
        javascript_code.add_child('code', """
            function update_image%(id)s(){
                var url = '/%(id)s/get_image_data';
                var xhr = new XMLHttpRequest();
                xhr.open('GET', url, true);
                xhr.responseType = 'blob'
                xhr.onload = function(e){
                    var urlCreator = window.URL || window.webkitURL;
                    var imageUrl = urlCreator.createObjectURL(this.response);
                    document.getElementById('%(id)s').src = imageUrl;
                }
                xhr.send();
            };

            setInterval( update_image%(id)s, %(update_rate)s );
            """ % {'id': id(self), 'update_rate': 1000.0 / self.fps})

        self.add_child('javascript_code', javascript_code)
   
    def get_image_data(self):
        global cameraFeedthrough
        global cameraProcessing
        global cameraKirsch
        if (self.mode == 0): # Grays
            self.frameLeftFeedthrough, self.frameRightFeedthrough = cameraFeedthrough.getStereoGray()
            self.frameLeftProcessed = cameraKirsch.getGray()
            self.frameRightProcessed = self.frameLeftProcessed
        elif (self.mode == 1): # Gaussian Smoothing
            self.frameLeftFeedthrough, self.frameRightFeedthrough = cameraFeedthrough.getStereoGray()
            self.frameLeftProcessed = cameraKirsch.getGaussianSmoothed()
            self.frameRightProcessed = self.frameLeftProcessed
        elif (self.mode == 2): # Ix Iy
            self.frameLeftFeedthrough, self.frameRightFeedthrough = cameraFeedthrough.getStereoGray()
            self.frameLeftProcessed, self.frameRightProcessed = cameraKirsch.getIxIy()
        elif (self.mode == 3): # Mag Theta
            self.frameLeftFeedthrough, self.frameRightFeedthrough = cameraFeedthrough.getStereoGray()
            self.frameLeftProcessed, self.frameRightProcessed = cameraKirsch.getMagTheta()
        elif (self.mode == 4): # Magnitude Filter
            self.frameLeftFeedthrough, self.frameRightFeedthrough = cameraFeedthrough.getStereoGray()
            self.frameLeftProcessed,_,_,_,_ = cameraKirsch.getBinaryImages()
            self.frameRightProcessed = self.frameLeftProcessed
        elif (self.mode == 5): # All Kirsch Filters
            _,self.frameLeftFeedthrough,self.frameRightFeedthrough,self.frameLeftProcessed,self.frameRightProcessed = cameraKirsch.getBinaryImages()
        else:
            assert(False)

        self.frameFeedthrough = np.concatenate((self.frameLeftFeedthrough,self.frameRightFeedthrough),axis=1)
        self.frameProcessed = np.concatenate((self.frameLeftProcessed,self.frameRightProcessed),axis=1)
        self.frame = np.concatenate((self.frameFeedthrough,self.frameProcessed),axis=0)
        ret,self.jpeg = cv2.imencode('.jpg', self.frame)
        return [self.jpeg.tostring(), self.headers]

class MyApp(App):
    def __init__(self, *args):
        self.f2RegMap = RegisterMap(1000, offset=0x81200000)
        # Fusion 2 Specific Registers
        self.f2RegMap.addRegister('IP Core Rst',     Register(0x000, size=1))
        self.f2RegMap.addRegister('IP Core En',      Register(0x004, size=4))
        self.f2RegMap.addRegister('Image Width',     Register(0x008, size=4, default=752))
        self.f2RegMap.addRegister('Image Height',    Register(0x00C, size=4, default=480))
        self.f2RegMap.addRegister('Horizontal Porch',Register(0x010, size=4, default=94))
        self.f2RegMap.addRegister('Vertical Porch',  Register(0x014, size=4, default=45))
        # Fusion 2 Hardware Target Registers
        self.f2RegMap.addRegister('kirschL',         Register(0x100, size=2, default=2))
        self.f2RegMap.addRegister('kirschS',         Register(0x104, size=2, default=1))
        self.f2RegMap.addRegister('magThr',          Register(0x108, size=2, default=1311))
        self.f2RegMap.addRegister('vCoefficients',   Register(0x110, size=4, count=3, default=[484494025, 1178495598, 484494025]))
        self.f2RegMap.addRegister('hCoefficients',   Register(0x130, size=4, count=3, default=[484494025, 1178495598, 484494025]))
        super(MyApp, self).__init__(*args)
        
    def main(self, name='world'):
        self.simulink = True
        if self.simulink == True:
            print("init")
            # Reset IP Core
            self.f2RegMap.writeRegister('IP Core Rst', 1)
            # Initialize defaults for registers
            self.f2RegMap.initRegisters()
            # Enable IP Core
            self.f2RegMap.writeRegister('IP Core En', 1)
        
        verticalContainer = gui.Container(width=1524, layout_orientation=gui.Container.LAYOUT_VERTICAL, margin='0px auto', style={'display': 'block', 'overflow': 'hidden'})
        cntrlContainer = gui.Container(width='100%', layout_orientation=gui.Container.LAYOUT_HORIZONTAL, margin='0px', style={'display': 'block', 'overflow': 'auto'})
        paramsContainer = gui.Container(width='100%', layout_orientation=gui.Container.LAYOUT_HORIZONTAL, margin='0px', style={'display': 'block', 'overflow': 'auto'})
        vidContainer = gui.Container(width='100%', layout_orientation=gui.Container.LAYOUT_HORIZONTAL, margin='0px', style={'display': 'block', 'overflow': 'auto'})

        self.videoDisplay = VideoDisplayWidget(10, width=1504, height=960)
        self.videoDisplay.style['margin'] = '10px'
        
        self.btn1 =     gui.Button('Capture', width=200, height=30, margin='10px')
        self.btn2 =     gui.Button('Mode', width=200, height=30, margin='10px')
        self.btn3 =     gui.Button('Close', width=200, height=30, margin='10px')

        self.btn1.onclick.do(self.on_button_pressed1)
        self.btn2.onclick.do(self.on_button_pressed2)
        self.btn3.onclick.do(self.on_button_pressed3)

        ## Configuration Parameters
        # Thresholding
        self.lblKirschThrL = gui.Label('Kirsch Threshold L', width=50, height=30, margin='10px')
        self.spinKirschThrL = gui.SpinBox(min=0, max=5000, width=50, height=30, margin='10px')
        self.spinKirschThrL.set_value(2)
        self.spinKirschThrL.onchange.do(self.spinChangedKirschThrL)
        
        self.lblKirschThrS = gui.Label('Kirsch Threshold S', width=50, height=30, margin='10px')
        self.spinKirschThrS = gui.SpinBox(min=0, max=5000, width=50, height=30, margin='10px')
        self.spinKirschThrS.set_value(1)
        self.spinKirschThrS.onchange.do(self.spinChangedKirschThrS)
        
        self.lblMagThr = gui.Label('Mag Threshold', width=50, height=30, margin='10px')
        self.spinMagThr = gui.SpinBox(min=0, max=5000, width=50, height=30, margin='10px')
        self.spinMagThr.set_value(0.04)
        self.spinMagThr.onchange.do(self.spinChangedMagThr)

        paramsContainer.append(self.lblKirschThrL)
        paramsContainer.append(self.spinKirschThrL)
        paramsContainer.append(self.lblKirschThrS)
        paramsContainer.append(self.spinKirschThrS)
        paramsContainer.append(self.lblMagThr)
        paramsContainer.append(self.spinMagThr)
        
        cntrlContainer.append(self.btn1)
        cntrlContainer.append(self.btn2)
        cntrlContainer.append(self.btn3)
        vidContainer.append(self.videoDisplay)
        verticalContainer.append(cntrlContainer)
        verticalContainer.append(paramsContainer)
        verticalContainer.append(vidContainer)
        return verticalContainer
      
    def on_button_pressed1(self, widget):
        i = 1
        mode = self.videoDisplay.mode

        if (mode == 0): # Grays
            filename = "gray%s.bmp"
            while os.path.exists(filename % i):
                i += 1
            cv2.imwrite("ftGray%s.bmp" % i, self.videoDisplay.frameLeftFeedthrough)
            cv2.imwrite(filename % i, self.videoDisplay.frameLeftProcessed)
        elif (mode == 1): # Gaussian Smoothing
            filename = "gaussian%s.bmp"
            while os.path.exists(filename % i):
                i += 1
            cv2.imwrite(filename % i, self.videoDisplay.frameLeftProcessed)
        elif (mode == 2): # Ix Iy
            filename_ix = "ix%s.bmp"
            filename_iy = "iy%s.bmp"
            while os.path.exists(filename_ix % i):
                i += 1
            cv2.imwrite(filename_ix % i, self.videoDisplay.frameLeftProcessed)
            cv2.imwrite(filename_iy % i, self.videoDisplay.frameRightProcessed)
        elif (mode == 3): # Mag Theta
            filename_mag = "mag%s.bmp"
            filename_theta = "theta%s.bmp"
            while os.path.exists(filename_mag % i):
                i += 1
            cv2.imwrite(filename_mag % i, self.videoDisplay.frameLeftProcessed)
            cv2.imwrite(filename_theta % i, self.videoDisplay.frameRightProcessed)
        elif (mode == 4): # Magnitude Filter
            filename = "magThr%s.bmp"
            while os.path.exists(filename % i):
                i += 1
            cv2.imwrite(filename % i, self.videoDisplay.frameLeftProcessed)
        elif (mode == 5): # All Kirsch Filters
            _,self.frameLeftProcessed,self.frameRightProcessed,self.frameLeftFeedthrough,self.frameRightFeedthrough = cameraKirsch.getBinaryImages()
            filename_n = "k_n%s.bmp"
            filename_nw = "k_nw%s.bmp"
            filename_w = "k_w%s.bmp"
            filename_sw = "k_sw%s.bmp"
            while os.path.exists(filename_n % i):
                i += 1
            cv2.imwrite(filename_n % i, self.videoDisplay.frameLeftFeedthrough)
            cv2.imwrite(filename_nw % i, self.videoDisplay.frameRightFeedthrough)
            cv2.imwrite(filename_w % i, self.videoDisplay.frameLeftProcessed)
            cv2.imwrite(filename_sw % i, self.videoDisplay.frameRightProcessed)
        else:
            assert(False)

    def on_button_pressed2(self, widget):
        self.videoDisplay.mode = (self.videoDisplay.mode + 1) % 6
 
    def on_button_pressed3(self, _):
        if self.simulink == True:
            self.simulinkMem.close()
            self.f1.close()
        self.close()  # closes the application

    def spinChangedKirschThrL(self, widget, value):
        if self.simulink == True:
            self.f2RegMap.writeRegister('kirschL', value)

    def spinChangedKirschThrS(self, widget, value):
        if self.simulink == True:
            self.f2RegMap.writeRegister('kirschS', value)
    
    def spinChangedMagThr(self, widget, value):
        if self.simulink == True:
            self.f2RegMap.writeRegister('magThr', fpu.FloatToQX_X(value, 15))

    def on_close(self):
        print("closing server")
        super(MyApp, self).on_close()

    # This is required to override the BaseHTTPRequestHandler logger
    def log_message(self, *args, **kwargs):
        pass
        
if __name__ == "__main__":
    #logging.getLogger('remi').setLevel(logging.WARNING)
    #logging.getLogger('remi').disabled = True
    #logging.getLogger('remi.server.ws').disabled = True
    #logging.getLogger('remi.server').disabled = True
    #logging.getLogger('remi.request').disabled = True
    start(MyApp, debug=False, address='0.0.0.0', port=8081, start_browser=False, multiple_instance=False)
