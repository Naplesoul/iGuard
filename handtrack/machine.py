
from typing import Tuple
import cv2
import numpy as np

red_min = np.array([0, 151, 100])
red_max = np.array([255, 255, 255])


def calibrate(img):
    detect_img = cv2.cvtColor(img, cv2.COLOR_BGR2LAB)
    red_img = cv2.inRange(detect_img, red_min, red_max)
    contours, _ = cv2.findContours(red_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    for contour in contours:
        rotated_rect = cv2.minAreaRect(contour)
        area = rotated_rect[1][0] * rotated_rect[1][1]
        if area < 100:
            continue
        center_x = int(rotated_rect[0][0])
        center_y = int(rotated_rect[0][1])
        print(center_x, center_y, area)


def detect(color_image):
    detect_img = cv2.cvtColor(color_image, cv2.COLOR_BGR2LAB)
    red_img = cv2.inRange(detect_img, red_min, red_max)
    contours, _ = cv2.findContours(red_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    for contour in contours:
        rotated_rect = cv2.minAreaRect(contour)
        area = rotated_rect[1][0] * rotated_rect[1][1]
        if area < 100:
            continue
        center_x = int(rotated_rect[0][0])
        center_y = int(rotated_rect[0][1])
        print(center_x, center_y, area)
    
    machine_info = { "x": 0, "y": 0, "running": False }
    return machine_info