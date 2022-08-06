import json
import socket
import numpy as np
from math import cos, sin
from scipy.optimize import root

listen_port = 50008
idx = 4
opt_idx = [[0,1],[0,2],[0,3],[1,2],[1,3],[2,3]]

real = np.asarray([[400, 0, 0, 400],
                   [0, 0, 0, 0],
                   [0, 0, 400, 800],
                   [1, 1, 1, 1]])

cam = None

def getCam(recv: socket.socket):
    global cam

    bytes, addr = recv.recvfrom(10240)
    payload = json.loads(bytes.decode())
    cam = np.asarray(payload)


def getM(vec):
    r = vec[0]
    p = vec[1]
    h = vec[2]
    x = vec[3]
    y = vec[4]
    z = vec[5]
    M = [[cos(r) * cos(h) - sin(r) * sin(p) * sin(h), -sin(r) * cos(p), cos(r) * sin(h) + sin(r) * sin(p) * cos(h), x],
         [sin(r) * cos(h) + cos(r) * sin(p) * sin(h), cos(r) * cos(p), sin(r) * sin(h) - cos(r) * sin(p) * cos(h), y],
         [-cos(p) * sin(h), sin(p), cos(p) * cos(h), z],
         [0, 0, 0, 1]]
    return np.asarray(M)


def f(x):
    M = getM(x)
    res = M.dot(cam) - real
    return [res[0][opt_idx[idx][0]],
            res[1][opt_idx[idx][0]],
            res[2][opt_idx[idx][0]],
            res[0][opt_idx[idx][1]],
            res[1][opt_idx[idx][1]],
            res[2][opt_idx[idx][1]]]


if __name__ == "__main__":
    recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv.bind(("0.0.0.0", listen_port))
    while True:
        getCam(recv)
        print("cam:\n", cam)
        x = [0, 0, 0, 0, 0, 0]
        result = root(f, x)
        result = result.x
        print("result:\n", result)
        M = getM(result)
        # print("cal:\n", M.dot(cam))
        print(M.dot(cam) - real)
        print("\n\n")