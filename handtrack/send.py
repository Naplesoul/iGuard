import json
import socket
from threading import Thread
import numpy as np

ip = "127.0.0.1"
port = 50000
camera_id = 3
update_offset = None
poll_thread = None
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

M_inv = None


def init(id: int, server_ip: str, server_port: int, update, dir: list):
    global M_inv, ip, port, camera_id, update_offset, poll_thread

    ip = server_ip
    port = server_port
    camera_id = id
    update_offset = update

    dir_x = np.linalg.norm(dir[0])
    dir_y = np.linalg.norm(dir[1])
    dir_z = np.linalg.norm(dir[2])

    r3 = np.array([dir[0] / dir_x, dir[1] / dir_y, dir[2] / dir_z])
    R3_inv = np.linalg.inv(r3)
    R4_inv = np.identity(4)
    R4_inv[0][:3] = R3_inv[0]
    R4_inv[1][:3] = R3_inv[1]
    R4_inv[2][:3] = R3_inv[2]
    T_inv = np.identity(4)
    M_inv = np.dot(T_inv, R4_inv)
    print(f"M_inv:\n{M_inv}")
    poll_thread = Thread(target=recv)
    poll_thread.setDaemon(True)
    poll_thread.start()


def recv():
    while True:
        bytes, addr = sock.recvfrom(1024)
        payload = json.loads(bytes.decode())
        offset = payload["offset"]
        update_offset(offset)


def convert(pos: list) -> list:
    pos = np.array(pos) * 1000
    return np.dot(M_inv, pos)[:3].tolist()


def send(frame_id: int, left_hand_landmarks, right_hand_landmarks, left_score: float, right_score: float):
    left_hand = []
    right_hand = []
    payload = { "camera_id": camera_id, "frame_id": frame_id }

    for pos in left_hand_landmarks:
        left_hand.append(convert([-pos.x, -pos.y, pos.z, 1]))
    
    for pos in right_hand_landmarks:
        right_hand.append(convert([-pos.x, -pos.y, pos.z, 1]))
    
    length = len(left_hand)
    if length > 0:
        left_x = left_hand[0][0]
        left_y = left_hand[0][1]
        left_z = left_hand[0][2]

        left_hand[0][0] = int(0)
        left_hand[0][1] = int(0)
        left_hand[0][2] = int(0)

        for i in range(1, length):
            left_hand[i][0] = int(left_hand[i][0] - left_x)
            left_hand[i][1] = int(left_hand[i][1] - left_y)
            left_hand[i][2] = int(left_hand[i][2] - left_z)
        
        payload["left_hand_nodes"] = left_hand
        payload["left_hand_score"] = int(left_score * 100)
    
    length = len(right_hand)
    if length > 0:
        right_x = right_hand[0][0]
        right_y = right_hand[0][1]
        right_z = right_hand[0][2]

        right_hand[0][0] = int(0)
        right_hand[0][1] = int(0)
        right_hand[0][2] = int(0)

        for i in range(1, length):
            right_hand[i][0] = int(right_hand[i][0] - right_x)
            right_hand[i][1] = int(right_hand[i][1] - right_y)
            right_hand[i][2] = int(right_hand[i][2] - right_z)
        
        payload["right_hand_nodes"] = right_hand
        payload["right_hand_score"] = int(right_score * 100)

    payload_str = json.dumps(payload)
    print(payload_str)
    sock.sendto(payload_str.encode(), (ip, port))
