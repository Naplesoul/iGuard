import json
import socket
from time import time
import torch

import utils
import config

def get_skeleton(recv: socket.socket) -> list:
    bytes, addr = recv.recvfrom(10240)
    node_info = json.loads(bytes.decode())

    skeleton = []
    for i in range(25):
        if i in [0, 10, 11, 16, 20, 24]:
            continue
        skeleton.append([node_info["nodes"][i]["x"], node_info["nodes"][i]["y"], node_info["nodes"][i]["z"]])
    return skeleton


if __name__ == "__main__":
    recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv.bind(("0.0.0.0", config.skeleton_port))
    send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    epoch, model, optimizer = utils.load_model(config.use_cuda)
    model = model.eval()

    serial = []

    while len(serial) < config.input_length:
        serial.append(get_skeleton(recv))

    if config.use_cuda:
        while True:
            serial = serial[1:]
            serial.append(get_skeleton(recv))

            start = time()

            input = torch.tensor(serial)
            input = input.type(torch.cuda.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)
            output = float(model(input)[0])

            send.sendto(json.dumps({ "value": output }).encode(), (config.similarity_ip, config.similarity_port))

            end = time()
            print("result: {:.2f}\tinfer time: {} ms".format(output, int((end - start) * 1000)))

    else:
        while True:
            serial = serial[1:]
            serial.append(get_skeleton(recv))

            start = time()

            input = torch.tensor(serial)
            input = input.type(torch.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)
            output = float(model(input)[0])

            send.sendto(json.dumps({ "value": output }).encode(), (config.similarity_ip, config.similarity_port))

            end = time()
            print("result: {:.2f}\tinfer time: {} ms".format(output, int((end - start) * 1000)))
