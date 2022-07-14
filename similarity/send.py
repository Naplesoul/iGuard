import os
import json
import socket
import sys
from time import sleep

import config

data_dir = config.processed_dir
framerate = config.model_framerate

ip = config.gui_ip
port = config.gui_port

if __name__ == "__main__":
    send_serial_id = int(sys.argv[len(sys.argv) - 1])
    motion_name = sys.argv[len(sys.argv) - 2]
    f = open(os.path.join(data_dir, motion_name + ".json"), "r")
    motion = json.load(f)
    f.close()
    serial = motion[send_serial_id]

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    for i in range(len(serial)):
        frame = serial[i]
        nodes = [{"x": x, "y": y, "z": z, "score": 0.75} for (x, y, z) in tuple(frame)]
        pack = {
            "body_nodes": nodes,
            "frame_id": i
        }
        s.sendto(json.dumps(pack).encode(), (ip, port))
        sleep(1 / framerate)