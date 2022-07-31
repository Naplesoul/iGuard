import json
import socket
import numpy as np
from typing import Dict
from threading import Thread

ip = "127.0.0.1"
port = 50000
camera_id = 3
poll_thread = None
update_offset = None
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def init(id: int, server_ip: str, server_port: int, update):
    global ip, port, camera_id, update_offset, poll_thread

    ip = server_ip
    port = server_port
    camera_id = id
    update_offset = update

    poll_thread = Thread(target=recv)
    poll_thread.setDaemon(True)
    poll_thread.start()


def recv():
    while True:
        bytes, addr = sock.recvfrom(1024)
        payload = json.loads(bytes.decode())
        offset = payload["offset"]
        update_offset(offset)


def send(frame_id: int, machine_info: Dict, hands_info: Dict):
    payload = { "camera_id": camera_id, "frame_id": frame_id }

    for key in machine_info:
        payload[key] = machine_info[key]

    for key in hands_info:
        payload[key] = hands_info[key]

    payload_str = json.dumps(payload)
    print(payload_str)
    sock.sendto(payload_str.encode(), (ip, port))
