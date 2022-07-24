import cv2
import numpy as np

red_min = np.array([0, 160, 100], dtype="uint8")
red_max = np.array([255, 255, 255], dtype="uint8")

green_min = np.array([0, 0, 100], dtype="uint8")
green_max = np.array([255, 108, 200], dtype="uint8")

area_min = 100
width_min = 6

switch = [0, 0, 0, 0]
carriage_lower_left = [0, 0, 0, 0]
carriage_upper_left = [0, 0, 0, 0]
carriage_upper_right = [0, 0, 0, 0]
carriage_lower_right = [0, 0, 0, 0]

range_y = 0
left_vector = [0, 0]
right_vector = [0, 0]
around_switch = [[0, 0]]

carriage_x = 0
carriage_z = 0
running = False
carriage_x_offset = 7

def init(calibration):
    global switch, around_switch
    global carriage_lower_left
    global carriage_upper_left
    global carriage_upper_right
    global carriage_lower_right
    global range_y, left_vector, right_vector

    switch = calibration["switch"]
    around_switch = []
    around_switch.append([switch[0] - 1, switch[1] - 1])
    around_switch.append([switch[0] - 1, switch[1]])
    around_switch.append([switch[0] - 1, switch[1] + 1])
    around_switch.append([switch[0], switch[1] - 1])
    around_switch.append([switch[0], switch[1]])
    around_switch.append([switch[0], switch[1] + 1])
    around_switch.append([switch[0] + 1, switch[1] - 1])
    around_switch.append([switch[0] + 1, switch[1]])
    around_switch.append([switch[0] + 1, switch[1] + 1])

    carriage_lower_left = calibration["carriage_lower_left"]
    carriage_upper_left = calibration["carriage_upper_left"]
    carriage_upper_right = calibration["carriage_upper_right"]
    carriage_lower_right = calibration["carriage_lower_right"]
    range_y = carriage_lower_left[1] - carriage_upper_left[1]
    left_vector = [carriage_lower_left[0] - carriage_upper_left[0], carriage_lower_left[1] - carriage_upper_left[1]]
    right_vector = [carriage_lower_right[0] - carriage_upper_right[0], carriage_lower_right[1] - carriage_upper_right[1]]


def find(lab_image):
    red_img = cv2.inRange(lab_image, red_min, red_max)
    # cv2.imwrite("red.png", red_img)
    # cv2.imwrite("green.png", cv2.inRange(lab_image, green_min, green_max))
    contours_to_select, _ = cv2.findContours(red_img, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    contours = []
    for contour in contours_to_select:
        rotated_rect = cv2.minAreaRect(contour)
        w = int(rotated_rect[1][0])
        h = int(rotated_rect[1][1])
        area = w * h

        # print(rotated_rect, lab_image[int(rotated_rect[0][1])][int(rotated_rect[0][0])], area)
        if area < area_min or w < width_min or h < width_min:
            continue
        x = int(rotated_rect[0][0])
        y = int(rotated_rect[0][1])
        contours.append([x, y, w, h])
    
    return sorted(contours, key=lambda k: k[0])


def convert(carriage):
    global carriage_upper_left
    global carriage_upper_right

    ratio = (carriage[1] - carriage_upper_left[1]) / range_y
    mid_left_x = carriage_upper_left[0] + left_vector[0] * ratio
    mid_right_x = carriage_upper_right[0] + right_vector[0] * ratio
    x = int((carriage[0] - mid_left_x) / (mid_right_x - mid_left_x) * 100)
    z = int(-ratio * 100)
    return x, z


def detect(bgr_image):
    global carriage_x, carriage_z, running

    lab_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)

    for pixel in around_switch:
        color = cv2.inRange(lab_img[pixel[1]][pixel[0]], green_min, green_max)
        if color[0] == 255 and color[1] == 255 and color[2] == 255:
            # green color
            running = True
            break
        color = cv2.inRange(lab_img[pixel[1]][pixel[0]], red_min, red_max)
        if color[0] == 255 and color[1] == 255 and color[2] == 255:
            # red color
            running = False
            break

    rects = find(lab_img)
    if len(rects) < 1:
        return { "carriage_x": carriage_x + carriage_x_offset, "carriage_z": carriage_z, "running": running }
    
    x, z = convert(rects[0])
    
    if abs(x - carriage_x) > 30 or abs(z - carriage_z) > 30:
        return { "carriage_x": carriage_x + carriage_x_offset, "carriage_z": carriage_z, "running": running }
    
    carriage_x = x
    carriage_z = z
    return { "carriage_x": x + carriage_x_offset, "carriage_z": z, "running": running }
