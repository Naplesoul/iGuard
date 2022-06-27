import json
import torch

import utils

def load_json(filename):
    f = open(filename,'r')
    data = json.load(f)
    f.close()
    return data

if __name__ == "__main__":
    data = load_json("./dataset/1.json")
    data = data[:100]
    epoch, model, optimizer = utils.load_model()
    model = model.eval().cuda()

    input = torch.tensor(data)
    input = input.type(torch.cuda.DoubleTensor).squeeze().view(-1,50,34).permute(1,0,2)
    output = model(input)
    print(output)