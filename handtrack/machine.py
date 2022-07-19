import cv2
import numpy as np

red_min = np.array([0, 151, 100])
red_max = np.array([255, 255, 255])

width = 640
height = 480

range_x = 400
range_z = 200

carriage_origin_x = 640
carriage_origin_y = 0
carriage_threshold = 0

switch_origin_x = 0
switch_origin_y = 0
switch_threshold = 0


def init(x, z):
    global range_x, range_z
    range_x = x
    range_z = z


def reset():
    global width, height
    global carriage_origin_x, carriage_origin_y, carriage_threshold
    global switch_origin_x, switch_origin_y, switch_threshold
    
    width = 640
    height = 480
    
    carriage_origin_x = 640
    carriage_origin_y = 0
    carriage_threshold = 0

    switch_origin_x = 0
    switch_origin_y = 0
    switch_threshold = 0


def calibrate(bgr_image) -> bool:
    global width, height
    global carriage_origin_x, carriage_origin_y, carriage_threshold
    global switch_origin_x, switch_origin_y, switch_threshold

    height = bgr_image.shape[0]
    width = bgr_image.shape[1]
    
    detect_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
    red_img = cv2.inRange(detect_img, red_min, red_max)
    contours, _ = cv2.findContours(red_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    contours_num = 0
    for contour in contours:
        rotated_rect = cv2.minAreaRect(contour)
        area = rotated_rect[1][0] * rotated_rect[1][1]
        if area < 100:
            continue

        contours_num += 1
        center_x = rotated_rect[0][0]
        center_y = rotated_rect[0][1]
        
        if center_x < carriage_origin_x:
            carriage_origin_x = center_x
            carriage_origin_y = center_y
            carriage_threshold = 2 * max(rotated_rect[1][0], rotated_rect[1][1])
        
        if center_x > switch_origin_x:
            switch_origin_x = center_x
            switch_origin_y = center_y
            switch_threshold = 2 * max(rotated_rect[1][0], rotated_rect[1][1])
    
    if contours_num < 2:
        print("Calibration failed: cannot find enough contours.")
        reset()
        return False
    
    threshold = max(carriage_threshold, switch_threshold)
    if abs(carriage_origin_x - switch_origin_x) < threshold and abs(carriage_origin_y - switch_origin_y) < threshold:
        print("Calibration failed: contours too close")
        print(f"Carriage ({int(carriage_origin_x)}, {carriage_origin_y}) Switch ({int(switch_origin_x)}, {int(switch_origin_y)})")
        reset()
        return False

    print(f"Calibrated: Carriage ({int(carriage_origin_x)}, {carriage_origin_y}) Switch ({int(switch_origin_x)}, {int(switch_origin_y)})")
    return True


def detect(bgr_image):
    detect_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
    red_img = cv2.inRange(detect_img, red_min, red_max)
    contours, _ = cv2.findContours(red_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    carriage_x = 640
    carriage_y = 0

    switch_x = 0
    switch_y = 0

    for contour in contours:
        rotated_rect = cv2.minAreaRect(contour)
        area = rotated_rect[1][0] * rotated_rect[1][1]
        if area < 100:
            continue
        center_x = rotated_rect[0][0]
        center_y = rotated_rect[0][1]

        # print(center_x, center_y, detect_img[int(center_y)][int(center_x)])

        if center_x < carriage_x:
            carriage_x = center_x
            carriage_y = center_y
        
        if center_x > switch_x:
            switch_x = center_x
            switch_y = center_y
    
    machine_running = abs(switch_x - switch_origin_x) > switch_threshold or abs(switch_y - switch_origin_y) > switch_threshold
    
    x = int((carriage_x - carriage_origin_x) / width * range_x)
    z = int((carriage_origin_y - carriage_y) / height * range_z)
    machine_info = { "carriage_x": x, "carriage_z": z, "running": machine_running }
    return machine_info