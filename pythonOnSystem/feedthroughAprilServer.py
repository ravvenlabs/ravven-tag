# Fusion 2 Feedthrough Server
import cv2
import remi.gui as gui
from remi import start, App
import logging
import numpy as np
import struct
import ctypes
import apriltag

from backend.grabbers.ImageFeedthrough import ImageFeedthrough

global camera
camera = ImageFeedthrough()

class VideoDisplayWidget(gui.Image):
    def __init__(self,fps=5, **kwargs):
        super(VideoDisplayWidget, self).__init__("/%s/get_image_data" % id(self), **kwargs)
        self.flag = True
        self.fps = fps
        self.detector = apriltag.Detector()
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
        global camera
        self.frameLeft,self.frameRight = camera.getStereoAll()
        tempImageLeft = np.ascontiguousarray(self.frameLeft[:,:,0:3], dtype=np.uint8)   # must make it contiguous for opencv processing to work
        tempImageRight = np.ascontiguousarray(self.frameRight[:,:,0:3], dtype=np.uint8)   # must make it contiguous for opencv processing to work

        grayImageLeft = self.frameLeft[:,:,3]  
        grayImageRight = self.frameRight[:,:,3]   

        resultLeft = self.detector.detect(grayImageLeft)
        resultRight = self.detector.detect(grayImageRight)

        print resultLeft
        print resultRight

        if not resultLeft:
            # no detections
            pass
        else:
            x1 = int(resultLeft[0].corners[0][0])
            y1 = int(resultLeft[0].corners[0][1])
            x2 = int(resultLeft[0].corners[1][0])
            y2 = int(resultLeft[0].corners[1][1])
            x3 = int(resultLeft[0].corners[2][0])
            y3 = int(resultLeft[0].corners[2][1])
            x4 = int(resultLeft[0].corners[3][0])
            y4 = int(resultLeft[0].corners[3][1])
            cv2.line(tempImageLeft,(x1,y1),(x2,y2),(255,255,0),3)
            cv2.line(tempImageLeft,(x2,y2),(x3,y3),(255,255,0),3)
            cv2.line(tempImageLeft,(x3,y3),(x4,y4),(255,255,0),3)
            cv2.line(tempImageLeft,(x4,y4),(x1,y1),(255,255,0),3)

        resultRight = self.detector.detect(grayImageRight)

        if not resultRight:
            # no detections
            pass
        else:
            x1 = int(resultRight[0].corners[0][0])
            y1 = int(resultRight[0].corners[0][1])
            x2 = int(resultRight[0].corners[1][0])
            y2 = int(resultRight[0].corners[1][1])
            x3 = int(resultRight[0].corners[2][0])
            y3 = int(resultRight[0].corners[2][1])
            x4 = int(resultRight[0].corners[3][0])
            y4 = int(resultRight[0].corners[3][1])
            cv2.line(tempImageRight,(x1,y1),(x2,y2),(255,255,0),3)
            cv2.line(tempImageRight,(x2,y2),(x3,y3),(255,255,0),3)
            cv2.line(tempImageRight,(x3,y3),(x4,y4),(255,255,0),3)
            cv2.line(tempImageRight,(x4,y4),(x1,y1),(255,255,0),3)

        self.frame = np.concatenate((tempImageLeft,tempImageRight),axis=1)
        ret,self.jpeg = cv2.imencode('.jpg', self.frame)
        return [self.jpeg.tostring(), self.headers]

class MyApp(App):
    def __init__(self, *args):
        super(MyApp, self).__init__(*args)
        
    def main(self, name='world'):
        verticalContainer = gui.Container(width=1524, margin='0px auto', style={'display': 'block', 'overflow': 'hidden'})
        horizontalContainer = gui.Container(width='100%', layout_orientation=gui.Container.LAYOUT_HORIZONTAL, margin='0px', style={'display': 'block', 'overflow': 'auto'})

        self.videoDisplay = VideoDisplayWidget(1, width=1504, height=480)
        self.videoDisplay.style['margin'] = '10px'
        
        self.bt1 = gui.Button('Capture', width=200, height=30, margin='10px')
        self.bt2 = gui.Button('Mode', width=200, height=30, margin='10px')
        self.bt3 = gui.Button('Close', width=200, height=30, margin='10px')

        self.bt1.onclick.do(self.on_button_pressed1)
        self.bt2.onclick.do(self.on_button_pressed2)
        self.bt3.onclick.do(self.on_button_pressed3)
        
        horizontalContainer.append(self.videoDisplay)
        verticalContainer.append(self.bt1)
        verticalContainer.append(self.bt2)
        verticalContainer.append(self.bt3)
        verticalContainer.append(horizontalContainer)
        return verticalContainer
      
    def on_button_pressed1(self, widget):
        i = 1
        while os.path.exists("left%s.jpg" % i):
            i += 1
        
        saveString = "Saved Image Set: " + str(i)
        #self.bt1.set_text(str(i))
        self.bt1.set_text(saveString)
                
        cv2.imwrite("left%s.jpg" % i, self.videoDisplay.frameLeft)
        cv2.imwrite("right%s.jpg" % i, self.videoDisplay.frameRight)

    def on_button_pressed2(self, widget):
        self.videoDisplay.flag = not self.videoDisplay.flag
        
    def on_button_pressed3(self, _):
        self.close()  # closes the application

    def on_close(self):
        print("closing server")
        super(MyApp, self).on_close()

    #this is required to override the BaseHTTPRequestHandler logger
    def log_message(self, *args, **kwargs):
        pass
        
if __name__ == "__main__":
    logging.getLogger('remi').setLevel(logging.WARNING)
    logging.getLogger('remi').disabled = True
    logging.getLogger('remi.server.ws').disabled = True
    logging.getLogger('remi.server').disabled = True
    logging.getLogger('remi.request').disabled = True
    start(MyApp, debug=False, address='0.0.0.0', port=8081, start_browser=False, multiple_instance=False)