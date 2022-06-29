import os
import json
import socket
from time import sleep

data_dir = "./dataset/processed"
motion_name = "walk"
framerate = 120
send_serial_id = 0

ip = "192.168.0.111"
port = 50002

if __name__ == "__main__":
    f = open(os.path.join(data_dir, motion_name + ".json"), "r")
    motion = json.load(f)
    f.close()
    serial = motion[motion_name][send_serial_id]

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