import json
import os
import sys
import random
from time import time
import torch

import utils
import config

def load_json(filename):
    f = open(filename,'r')
    data = json.load(f)
    f.close()
    return data

if __name__ == "__main__":
    motion = sys.argv[len(sys.argv) - 1]
    if motion.isdigit():
        data = [load_json(os.path.join(config.record_dir, motion + ".json"))]
    else:
        data = load_json(os.path.join(config.processed_dir, motion + ".json"))

    for i in range(len(data)):
        serial_len = len(data[i])
        max_serial_start_idx = serial_len - config.input_length_sec * config.model_framerate
        start_idx = random.randint(0, max_serial_start_idx)
        data[i] = data[i][start_idx : start_idx + config.input_length_sec * config.model_framerate]

    epoch, model, optimizer = utils.load_model(config.use_cuda)
    model = model.eval()

    test_cnt = 20
    total_time = 0
    if config.use_cuda:
        for _ in range(test_cnt):
            start = time()
            input = torch.tensor(data)
            input = input.type(torch.cuda.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)
            output = model(input)
            end = time()
            total_time += end - start
    else:
        for _ in range(test_cnt):
            start = time()
            input = torch.tensor(data)
            input = input.type(torch.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)
            output = model(input)
            end = time()
            total_time += end - start
            
    print("output:", output)
    print("avg infer latency: {} ms".format(int(total_time * 1000 / test_cnt)))
