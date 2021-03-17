#!/usr/bin/python/

import cv2 as cv

from AprilTagDetector import AprilTagDetector

if __name__ == "__main__":
    detector = AprilTagDetector()
    start = cv.imread("/fusion2/start.png")
    mag = cv.imread("/fusion2/mag.png")
    theta = cv.imread("/fusion2/theta.png")
    print("Extracting normal tags")
    norm = detector.extractTags(start, True)
    print("Extracting mag theta tags")
    shortcut = list(detector.extractMagThetaTags(start, mag, theta, True))
    print(norm)
    print(shortcut)
    print(shortcut[0].corners[0].x)
    print(shortcut[0].corners[0].y)
