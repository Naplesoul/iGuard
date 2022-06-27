import numbers
import os
import json
from tokenize import Number
import torch
import numpy as np

import config
from net import MMD_NCA_Net

class NumpyEncoder(json.JSONEncoder):
    """ Special json encoder for numpy types """
    def default(self, obj):
        if isinstance(obj, (np.int_, np.intc, np.intp, np.int8,
            np.int16, np.int32, np.int64, np.uint8,
            np.uint16, np.uint32, np.uint64)):
            return int(obj)
        elif isinstance(obj, (np.float_, np.float16, np.float32, 
            np.float64)):
            return float(obj)
        elif isinstance(obj,(np.ndarray,)): #### This is the fix
            return obj.tolist()
        return json.JSONEncoder.default(self, obj) 
    
def save_to_json(dic,target_dir):
    dumped = json.dumps(dic, cls=NumpyEncoder)  
    file = open(target_dir, 'w')  
    json.dump(dumped, file)
    file.close()
    
def read_from_json(target_dir):
    f = open(target_dir,'r')
    data = json.load(f)
    # data = json.loads(data)
    f.close()
    return data

def save_model(epoch, model: MMD_NCA_Net, optimizer: torch.optim.Adam):
    print("saving epoch {}...".format(epoch))
    checkpoint = "{}.pth".format(epoch)
    torch.save({"epoch": epoch, "checkpoint": checkpoint}, os.path.join(config.model_dir, "model.meta"))
    torch.save({"net": model.state_dict(), "optimizer": optimizer.state_dict()}, os.path.join(config.model_dir, checkpoint))

def load_model():
    start_epoch = 0
    model = MMD_NCA_Net().cuda().double()

    if os.path.exists(os.path.join(config.model_dir, "model.meta")):
        meta = torch.load(config.model_dir + "model.meta")
        epoch = meta["epoch"]
        checkpoint_filename = meta["checkpoint"]
        checkpoint = torch.load(os.path.join(config.model_dir, checkpoint_filename))

        model.load_state_dict(checkpoint["net"])
        optimizer = torch.optim.Adam(model.parameters(), lr=config.learning_rate)
        optimizer.load_state_dict(checkpoint["optimizer"])

        print("loaded from epoch {}".format(epoch))
        start_epoch = epoch + 1

    else:
        optimizer = torch.optim.Adam(model.parameters(), lr=config.learning_rate)
    
    return (start_epoch, model, optimizer)