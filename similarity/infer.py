import json
import os
import sys
import random
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
    data = load_json(os.path.join(config.processed_dir, motion + ".json"))

    for i in range(len(data)):
        serial_len = len(data[i])
        max_serial_start_idx = serial_len - config.input_length_sec * config.model_framerate
        start_idx = random.randint(0, max_serial_start_idx)
        data[i] = data[i][start_idx, start_idx + config.input_length_sec * config.model_framerate]

    epoch, model, optimizer = utils.load_model()
    model = model.eval().cuda()

    input = torch.tensor(data)
    input = input.type(torch.cuda.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)
    output = model(input)
    print(output)