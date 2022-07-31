import cv2
import numpy as np
import mediapipe as mp
from typing import Dict

M_inv = None
mpHands = None


def init(dir: list):
    global M_inv, mpHands
    
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

    mpHands = mp.solutions.hands.Hands()


def convert(pos: list) -> list:
    pos = np.array(pos) * 1000
    return np.dot(M_inv, pos)[:3].tolist()


def process(left_hand_landmarks, right_hand_landmarks, left_score: float, right_score: float) -> Dict:
    left_hand = []
    right_hand = []
    payload = {}

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
    
    return payload


def detect(bgr_image) -> Dict:
    rgb = cv2.cvtColor(cv2.flip(bgr_image, 1), cv2.COLOR_BGR2RGB)

    left_hand = []
    right_hand = []
    left_score = 0
    right_score = 0

    results = mpHands.process(rgb)
    if not results.multi_hand_world_landmarks:
        print("No Hands found")
        return {}
        
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

    return process(left_hand, right_hand, left_score, right_score)