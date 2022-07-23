import sys
import json
import cv2

import camera
import machine


if __name__ == "__main__":
    config_filename = sys.argv[len(sys.argv) - 1]
    config_file = open(config_filename)
    config = json.load(config_file)
    config_file.close()

    serial = config["serial"]
    camera.init(serial)
    config = {}
    config_filename = "calibration.json"
    input("Calibration start!\nPlease place the carriage to lower left and press ENTER")
    for i in range(5):
        camera.get_color_image()
    while True:
        bgr_image = camera.get_color_image()
        lab_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
        rects = machine.find(lab_img)
        cv2.imwrite("lower_left.png", bgr_image)
        if len(rects) < 2:
            print("Cannot find enough (2) contours.")
            continue
        w = lab_img.shape[1]
        h = lab_img.shape[0]
        switch_x = max(rects[-1][0], 1)
        switch_y = max(rects[-1][1], 1)
        switch_x = min(switch_x, w - 2)
        switch_y = min(switch_y, h - 2)
        config["switch"] = [switch_x, switch_y, rects[-1][2], rects[-1][3]]
        config["carriage_lower_left"] = rects[-2]
        break

    input("Last step is OK!\nPlease place the carriage to upper left and press ENTER")
    for i in range(5):
        camera.get_color_image()
    while True:
        bgr_image = camera.get_color_image()
        lab_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
        rects = machine.find(lab_img)
        cv2.imwrite("upper_left.png", bgr_image)
        if len(rects) < 2:
            print("Cannot find enough (2) contours.")
            continue
        config["carriage_upper_left"] = rects[-2]
        break

    input("Last step is OK!\nPlease place the carriage to upper right and press ENTER")
    for i in range(5):
        camera.get_color_image()
    while True:
        bgr_image = camera.get_color_image()
        lab_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
        rects = machine.find(lab_img)
        cv2.imwrite("upper_right.png", bgr_image)
        if len(rects) < 2:
            print("Cannot find enough (2) contours.")
            continue
        config["carriage_upper_right"] = rects[-2]
        break

    input("Last step is OK!\nPlease place the carriage to lower right and press ENTER")
    for i in range(5):
        camera.get_color_image()
    while True:
        bgr_image = camera.get_color_image()
        lab_img = cv2.cvtColor(bgr_image, cv2.COLOR_BGR2LAB)
        rects = machine.find(lab_img)
        cv2.imwrite("lower_right.png", bgr_image)
        if len(rects) != 1:
            print("Contours count (1) unsatisfied.")
            continue
        config["carriage_lower_right"] = rects[0]
        break

    config_file = open(config_filename, "w+")
    json.dump(config, config_file)
    config_file.close()
    print(f"Calibration success! Calibration info has been written into ./{config_filename}")