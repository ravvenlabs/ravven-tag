#!/usr/bin/python/

try:
    import libpyboostapriltags as AT
except:
    import sys
    import os
    sys.path.append("../build/lib")
    import libpyboostapriltags as AT

class AprilTagDetector:
    def __init__(self):
        # Configure general parameters
        self.globals = AT.Globals()
        self.cp = AT.CameraParameters()
        self.cp.tagSize = 0.166
        self.cp.fx = 600
        self.cp.fy = 600
        self.cp.px = 752/2
        self.cp.py = 480/2
        # Create internal tag detector
        self.detector = AT.TagDetector(self.globals.tagCodes36h11)

    def extractTags(self, image, debug = False):
        detections = AT.extractTags(self.detector, image)
        if debug:
            for detection in detections:
                AT.print_detection(self.cp, detection)
        return AprilTagDetectionDataFactory.create_detections(self, detections)

    def extractMagThetaTags(self, start, mag, theta, debug = False):
        detections = AT.extractMagThetaTags(self.detector, start, mag, theta)
        if debug:
            for detection in detections:
                AT.print_detection(self.cp, detection)
        return AprilTagDetectionDataFactory.create_detections(self, detections)

class AprilTagDetectionDataFactory:
    @staticmethod
    def create_detection(detector, cpp_tag_detection):
        return AT.break_out_detection(detector.cp, cpp_tag_detection)

    @staticmethod
    def create_detections(detector, cpp_tag_detection_vector):
        for cpp_tag_detection in cpp_tag_detection_vector:
            yield AprilTagDetectionDataFactory.create_detection(detector, cpp_tag_detection)
