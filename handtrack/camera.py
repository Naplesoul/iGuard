import sys
import time
import numpy as np
import pyrealsense2 as rs
# import cv2

stream_res_x = 640
stream_res_y = 480
stream_fps = 30

pipeline = None
align = None


def init(serial: str):
    global pipeline, align

    pipeline = rs.pipeline()
    config = rs.config()
    config.enable_stream(rs.stream.depth, stream_res_x, stream_res_y, rs.format.z16, stream_fps)
    config.enable_stream(rs.stream.color, stream_res_x, stream_res_y, rs.format.bgr8, stream_fps)
    # config.enable_stream(rs.stream.infrared, 1, stream_res_x, stream_res_y, rs.format.y8, stream_fps)
    profile = pipeline.start(config)

    align_to = rs.stream.color
    align = rs.align(align_to)

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
        sys.exit(-1)


def get_color_image():
    while True:
        # Get and align frames
        frames = pipeline.wait_for_frames()
        aligned_frames = align.process(frames)
        color_frame = aligned_frames.get_color_frame()
        # ir_frame = aligned_frames.get_infrared_frame(1)
        # cv2.imwrite("ir.png", np.asanyarray(ir_frame.get_data()))
        if color_frame:
            return np.asanyarray(color_frame.get_data())
        print("NO Color Frame!")
        time.sleep(0.01)
    