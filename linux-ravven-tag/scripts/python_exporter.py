#!/usr/bin/python/

import cv2 as cv
import csv
import time
import numpy as np

from AprilTagDetector import AprilTagDetector

if __name__ == "__main__":
    detector = AprilTagDetector()
    start = cv.imread("/fusion2/input1.bmp", 0)
    with open("/fusion2/bram.csv") as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        rows = sum(1 for row in csv_reader)
        csv_file.seek(0)
        data = np.zeros((rows, 3), dtype=np.single)
        for line, row in enumerate(csv_reader):
            data[line, 0] = row[0]
            data[line, 1] = row[1]
            data[line, 2] = row[2]
    
    t = time.time()
    print(list(detector.extractSegmentedTags(start, data)))
    print(time.time() - t)
