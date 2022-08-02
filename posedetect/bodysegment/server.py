import socket
import cv2
import numpy as np
import mediapipe as mp

mp_seg = mp.solutions.selfie_segmentation

listen_port = 50007

def recv_img(recv: socket.socket):
    bytes, addr = recv.recvfrom(65536)
    bytes = np.asarray(bytearray(bytes), dtype="uint8")
    img = cv2.imdecode(bytes, cv2.IMWRITE_JPEG_QUALITY)
    return img

if __name__ == "__main__":
    seg = mp_seg.SelfieSegmentation(model_selection=1)

    recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv.bind(("0.0.0.0", listen_port))
    print(f"openvino infer server listening on port {listen_port}")
    send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:
        img = recv_img(recv)
        img = cv2.cvtColor(cv2.flip(img, 1), cv2.COLOR_BGR2RGB)
        results = seg.process(img)
