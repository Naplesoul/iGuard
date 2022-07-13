import json
import math
import sys
import time
import cv2
import numpy as np
import datetime as dt
import mediapipe as mp
import pyrealsense2 as rs

import send

stream_res_x = 640
stream_res_y = 480
stream_fps = 30
fps = 10
frametime = 1 / fps
first_frametime = dt.datetime.strptime("2022-07-06 00:00:00", "%Y-%m-%d %H:%M:%S").timestamp()

def wait_next_frame() -> int:
    now = dt.datetime.today().timestamp()
    total_time = now - first_frametime
    frame_id = math.ceil(total_time / frametime)
    time.sleep(first_frametime + frame_id * frametime - now)
    return frame_id

if __name__ == "__main__":
    config_filename = sys.argv[len(sys.argv) - 1]
    config_file = open(config_filename)
    user_config = json.load(config_file)
    config_file.close()

    fps = user_config["fps"]
    frametime = 1 / fps
    serial = user_config["serial"]
    camera_id = user_config["camera_id"]

    send.init(camera_id, user_config["server_ip"], user_config["server_port"],
              [user_config["camera_direction_x"], user_config["camera_direction_y"], user_config["camera_direction_z"]])

    # ====== Realsense ======
    device = ""
    realsense_ctx = rs.context()
    for i in range(len(realsense_ctx.devices)):
        detected_camera = realsense_ctx.devices[i].get_info(rs.camera_info.serial_number)
        if detected_camera == serial:
            device = detected_camera
            break
    if device == "":
        print(f"Camera SN {serial} Not Found")
        exit(-1)
    
    pipeline = rs.pipeline()
    config = rs.config()
    config.enable_stream(rs.stream.depth, stream_res_x, stream_res_y, rs.format.z16, stream_fps)
    config.enable_stream(rs.stream.color, stream_res_x, stream_res_y, rs.format.bgr8, stream_fps)
    profile = pipeline.start(config)

    align_to = rs.stream.color
    align = rs.align(align_to)

    mpHands = mp.solutions.hands.Hands()

    while True:
        frame_id = wait_next_frame()
        start_time = dt.datetime.today().timestamp()

        # Get and align frames
        frames = pipeline.wait_for_frames()
        aligned_frames = align.process(frames)
        color_frame = aligned_frames.get_color_frame()
        if not color_frame:
            send.send(frame_id, [], [], 0, 0)
            print("No Hands found")
            continue

        color_image = np.asanyarray(color_frame.get_data())
        color_image = cv2.flip(color_image,1)
        color_images_rgb = cv2.cvtColor(color_image, cv2.COLOR_BGR2RGB)

        results = mpHands.process(color_images_rgb)
        if not results.multi_hand_world_landmarks:
            send.send(frame_id, [], [], 0, 0)
            print("No Hands found")
            continue
        
        left_hand = []
        right_hand = []
        left_score = 0
        right_score = 0
        for i in range(len(results.multi_handedness)):
            label = results.multi_handedness[i].classification[0].label
            if label == "Left":
                left_score = results.multi_handedness[i].classification[0].score
                left_hand = results.multi_hand_world_landmarks[i].landmark

            elif label == "Right":
                right_score = results.multi_handedness[i].classification[0].score
                right_hand = results.multi_hand_world_landmarks[i].landmark
            
            if left_score > 0 and right_score > 0:
                break
            
        send.send(frame_id, left_hand, right_hand, left_score, right_score)

        end_time = dt.datetime.today().timestamp()
        print(f"process time: {int((end_time - start_time) * 1000)}ms")
            