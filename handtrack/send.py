import json
import socket
import numpy as np

ip = "localhost"
port = 50000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

M_inv = None

def init(server_ip: str, server_port: int, dir: list):
    global M_inv, ip, port

    ip = server_ip
    port = server_port

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


def convert(pos: list) -> list:
    pos = np.array(pos) * 1000
    return np.dot(M_inv, pos)[:3].tolist()


def send(frame_id: int, left_hand_landmarks, right_hand_landmarks, left_score: float, right_score: float):
    left_hand = []
    right_hand = []

    for pos in left_hand_landmarks:
        left_hand.append(convert([-pos.x, -pos.y, pos.z, 1]))
    
    for pos in right_hand_landmarks:
        right_hand.append(convert([-pos.x, -pos.y, pos.z, 1]))
    
    length = len(left_hand)
    for i in range(length):
        left_hand[i][0] = int(left_hand[i][0] - left_hand[0][0])
        left_hand[i][1] = int(left_hand[i][1] - left_hand[0][1])
        left_hand[i][2] = int(left_hand[i][2] - left_hand[0][2])
    
    length = len(right_hand)
    for i in range(length):
        right_hand[i][0] = int(right_hand[i][0] - right_hand[0][0])
        right_hand[i][1] = int(right_hand[i][1] - right_hand[0][1])
        right_hand[i][2] = int(right_hand[i][2] - right_hand[0][2])

    payload = {
        "frame_id": frame_id,
        "left_hand": left_hand,
        "right_hand": right_hand,
        "left_score": int(left_score * 100),
        "right_score": int(right_score * 100),
    }

    payload_str = json.dumps(payload)
    print(payload_str)
    sock.sendto(payload_str.encode(), (ip, port))
