import os
import json
import socket
from time import time
from openvino.runtime import Core

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
    anchor = skeleton[4]
    new_skeleton = []
    for k in range(4):
        new_skeleton += [skeleton[k][0] - anchor[0], skeleton[k][1] - anchor[1], skeleton[k][2] - anchor[2]]
    for k in range(5, config.joint_size):
        new_skeleton += [skeleton[k][0] - anchor[0], skeleton[k][1] - anchor[1], skeleton[k][2] - anchor[2]]
    return [new_skeleton]


if __name__ == "__main__":
    core = Core()
    net = core.compile_model(os.path.join(config.model_dir, "similarity.xml"), "AUTO")

    recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv.bind(("0.0.0.0", config.listen_port))
    print(f"openvino infer server listening on port {config.listen_port}")
    send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    serial = []

    while len(serial) < config.input_length:
        serial.append(get_skeleton(recv))

    while True:
        serial = serial[1:]
        serial.append(get_skeleton(recv))

        start = time()

        output = net([serial])[net.outputs[0]]
        send.sendto(json.dumps({ "value": output }).encode(), (config.gui_ip, config.gui_sim_port))

        end = time()
        print("result: {:.2f}\tinfer time: {} us".format(output, int((end - start) * 1000000)))
