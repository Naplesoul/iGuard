import sys
import time
import json
import math
import threading
import datetime as dt

import send
import camera
import machine
import hand


fps = 10
offset = 0
last_frame_id = 0
frametime = 1 / fps
first_frametime = dt.datetime.strptime("2022-07-15 00:00:00", "%Y-%m-%d %H:%M:%S").timestamp()
lock = threading.Lock()


def update_offset(_offset):
    lock.acquire()
    global offset
    offset += _offset / 1000
    offset_copy = offset
    lock.release()
    print(f"update offset: {int(offset_copy * 1000)} ms")


def wait_next_frame() -> int:
    global last_frame_id

    now = dt.datetime.today().timestamp()
    total_time = now - first_frametime
    frame_id = math.ceil(total_time / frametime)
    if frame_id <= last_frame_id:
        frame_id += 1

    lock.acquire()
    sleep_time = first_frametime + frame_id * frametime - now + offset
    lock.release()

    if sleep_time > 0.005:
        time.sleep(sleep_time)
    last_frame_id = frame_id
    return frame_id


def main():
    global fps, frametime

    config_filename = sys.argv[len(sys.argv) - 1]
    config_file = open(config_filename)
    user_config = json.load(config_file)
    config_file.close()

    fps = user_config["fps"]
    frametime = 1 / fps
    serial = user_config["serial"]
    camera_id = user_config["camera_id"]

    send.init(camera_id, user_config["server_ip"], user_config["server_port"], update_offset)

    camera.init(serial)

    machine.init(user_config["range_x"], user_config["range_z"])
    for i in range(5):
        camera.get_color_image()
    while not machine.calibrate(camera.get_color_image()):
        pass
            
    hand.init([user_config["camera_direction_x"], user_config["camera_direction_y"], user_config["camera_direction_z"]])

    while True:
        frame_id = wait_next_frame()
        start_time = dt.datetime.today().timestamp()
        color_image = camera.get_color_image()

        machine_info = machine.detect(color_image)
        hands_info = hand.detect(color_image)
            
        send.send(frame_id, machine_info, hands_info)

        end_time = dt.datetime.today().timestamp()
        process_time = end_time - start_time
        print(f"process time: {int(process_time * 1000)} ms\n\n")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\tKeyboardInterrupt")
        sys.exit(-1)
