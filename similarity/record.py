import os
import sys
import json
import socket

import config

last_frame_id = 0

def get_skeleton(recv: socket.socket) -> list:
    global last_frame_id

    while True:
        bytes, addr = recv.recvfrom(10240)
        payload = json.loads(bytes.decode())
        frame_id = payload["frame_id"]
        print(frame_id)
        if frame_id > last_frame_id:
            last_frame_id = frame_id
            break

    skeleton = payload["body_nodes"]
    return skeleton


if __name__ == "__main__":
    motion = input("motion name? > ")
    frame_num = int(input("frame num? > "))
    dir = os.path.join(config.record_dir, motion)
    if not os.path.exists(dir):
        os.makedirs(dir)

    while 1:
        filename = input("filename? > ") + ".json"
        recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        recv.bind(("0.0.0.0", config.listen_port))
        path = os.path.join(dir, filename)
        print(f"start recording {frame_num} frames into {path}")

        result = []
        print("-" * frame_num, end="")
        sys.stdout.flush()
        for i in range(frame_num):
            result.append(get_skeleton(recv))
            print("\b" * frame_num + "#" * (i + 1) + "-" * (frame_num - i - 1), end="")
            sys.stdout.flush()
        
        f = open(path, "w+")
        json.dump(result, f)
        f.close()
        
        print("\nsave result to " + filename)
