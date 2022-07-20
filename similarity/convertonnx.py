import os
import json
import torch
import torch.nn
import onnx

import utils
import config


def load_json(filename):
    f = open(filename,'r')
    data = json.load(f)
    f.close()
    return data


if __name__ == "__main__":
    epoch, model, optimizer = utils.load_model(config.use_cuda)
    model = model.eval()

    input_names = ['input']
    output_names = ['output']

    data = load_json(os.path.join(config.processed_dir, "turn.json"))[0][:config.input_length_sec * config.model_framerate]
    input = torch.tensor(data)
    input = input.type(torch.DoubleTensor).squeeze().view(-1, config.input_length, config.feature_size).permute(1,0,2)

    torch.onnx.export(model, input, os.path.join(config.model_dir, "similarity.onnx"), input_names=input_names, output_names=output_names)