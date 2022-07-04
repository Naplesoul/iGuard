import os
import json
import socket
import sys
from time import sleep

import config

data_dir = config.processed_dir
framerate = config.model_framerate

ip = config.server_ip
port = config.server_port

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
        nodes.insert(0, {"x": 0, "y": 0, "z": 0, "score": 0.75})
        nodes.insert(10, {"x": 0, "y": 0, "z": 0, "score": 0.75})
        nodes.insert(11, nodes[5])
        nodes.insert(16, {"x": 0, "y": 0, "z": 0, "score": 0.75})
        nodes.insert(20, {"x": 0, "y": 0, "z": 0, "score": 0.75})
        nodes.insert(24, {"x": 0, "y": 0, "z": 0, "score": 0.75})
        pack = {
            "node_num": 25,
            "nodes": nodes,
            "camera_id": 1,
            "frame_id": i
        }
        s.sendto(json.dumps(pack).encode(), (ip, port))
        sleep(1 / framerate)