#!/usr/bin/python/

import cv2 as cv
import csv
import time
import numpy as np

from AprilTagDetector import AprilTagDetector

def read_gray_image(file_path):
    with open(file_path) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        im = np.zeros((480, 752), dtype=np.uint8)
        for i, row in enumerate(csv_reader):
            for j, data in enumerate(row):
                im[i, j] = data
    return im

def read_xyw_data(file_path):
    with open(file_path) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        rows = sum(1 for row in csv_reader)
        csv_file.seek(0)
        data = np.zeros((rows, 3), dtype=np.single)
        for line, row in enumerate(csv_reader):
            data[line, 0] = row[0]
            data[line, 1] = row[1]
            data[line, 2] = row[2]
    return data

def read_segments_data(file_path):
    with open(file_path) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        rows = sum(1 for row in csv_reader)
        csv_file.seek(0)
        data = np.zeros((rows, 5), dtype=np.single)
        for line, row in enumerate(csv_reader):
            data[line, 0] = row[0]
            data[line, 1] = row[1]
            data[line, 2] = row[2]
            data[line, 3] = row[3]
            data[line, 4] = row[4]
    return data

if __name__ == "__main__":
    detector = AprilTagDetector()
    for i in range(1, 21):
        prefix = "../../matlab/pics/data/"
        start_file = "gray{}.csv".format(i)
        seg_file = "bram{}.csv".format(i)

        print("Testing: " + str(i))

        start = read_gray_image(prefix + start_file)
        segs = read_segments_data(prefix + seg_file)

        t = time.time()
        print(list(detector.extractSegmentedTags(start, segs)))
        print(time.time() - t)
