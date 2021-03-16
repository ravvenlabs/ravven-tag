#!/usr/bin/python3/

import cv2 as cv

from AprilTagDetector import AprilTagDetector

if __name__ == "__main__":
    detector = AprilTagDetector()
    start = cv.imread("../build/bin/start.png")
    mag = cv.imread("../build/bin/mag.png")
    theta = cv.imread("../build/bin/theta.png")
    print("Extracting normal tags")
    norm = detector.extractTags(start, True)
    print("Extracting mag theta tags")
    shortcut = list(detector.extractMagThetaTags(start, mag, theta, True))
    print(norm)
    print(shortcut)
    print(shortcut[0].corners[0].x)
    print(shortcut[0].corners[0].y)
