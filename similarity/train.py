import gc
import os
import numpy as np
import torch
import torch.nn as nn
from torch.autograd import Variable
from torch.utils.data import Dataset
from torch.utils.data import DataLoader

import config
import utils

use_cuda = torch.cuda.is_available()
    
class MMD_NCA_loss(nn.Module):
    def __init__(self):
        super().__init__()

    def kernel_function(self, x1, x2):
        k1 = torch.exp(-torch.pow((x1 - x2), 2) / 2)
        k2 = torch.exp(-torch.pow((x1 - x2), 2) / 8)
        k4 = torch.exp(-torch.pow((x1 - x2), 2) / 32)
        k8 = torch.exp(-torch.pow((x1 - x2), 2) / 128)
        k16 = torch.exp(-torch.pow((x1 - x2), 2) / 512)
        k_sum = k1+k2+k4+k8+k16
        return k_sum
    
    def MMD(self, x, x_IID, y, y_IID):
        # x, x_IID's dimension: m*25,  y, y_IID's dimension: n*25
        m = x.size()[0]
        n = y.size()[0]
        x = x.view(m, 1, -1)
        x_square = x.repeat(1, m, 1)
        x_IID = x_IID.view(-1, m, 1)
        x_IID_square = x_IID.repeat(m, 1, 1)
        value_1 = torch.sum(self.kernel_function(x_square, x_IID_square)) / (m**2)
        y = y.view(1, n, -1)
        y_square = y.repeat(n, 1, 1)
        value_2 = torch.sum(self.kernel_function(x_square, y_square)) / (m*n)
        y_IID = y_IID.view(n, 1, -1)
        y_IID_square = y_IID.repeat(1, n, 1)
        value_3 = torch.sum(self.kernel_function(y_IID_square, y_square)) / (n**2)
        return value_1 - 2*value_2 + value_3
    
    def forward(self, x):
        x = x.view(7, 25)
        numerator = torch.exp(-self.MMD(x[0], x[0], x[1], x[1]))
        value_1 = torch.exp(-self.MMD(x[0], x[0], x[2], x[2]))
        value_2 = torch.exp(-self.MMD(x[0], x[0], x[3], x[3]))
        value_3 = torch.exp(-self.MMD(x[0], x[0], x[4], x[4]))
        value_4 = torch.exp(-self.MMD(x[0], x[0], x[5], x[5]))
        value_5 = torch.exp(-self.MMD(x[0], x[0], x[6], x[6]))
        denominator = value_1 + value_2 + value_3 + value_4 + value_5
        loss = torch.exp(- numerator / denominator)
        return loss

class MMD_NCA_Dataset(Dataset):
    def __init__(self, dir, num_MMD_NCA_Groups):
        self.raw = {}
        available_datasets = {}
        classes_all = []
        for motion_class in config.train_classes:
            path = os.path.join(dir, motion_class[0] + ".json")
            trials = utils.read_from_json(path)
            self.raw[motion_class[0]] = trials

            available_dataset = []
            for i in range(len(trials)):
                serial_len = len(trials[i])
                max_serial_start_idx = serial_len - config.input_length_sec * config.model_framerate
                for j in range(max_serial_start_idx):
                    # [trial_idx, serial_idx]
                    available_dataset.append([i, j])
            
            if len(available_dataset) <= 25:
                print("too few dataset: {}: {} available dataset".format(motion_class[0], len(available_dataset)))
                continue

            print("{}: {} available dataset".format(motion_class[0], len(available_dataset)))
            classes_all.append(motion_class[0])
            available_datasets[motion_class[0]] = available_dataset

        self.num_MMD_NCA_Groups = num_MMD_NCA_Groups
        gc.collect()

        # [[[index of motion * 25] * 7 classes] * num_MMD_NCA_Groups]
        self.MMD_NCA_Groups = []
        # [[pos_class, pos_class, neg_class_0, ..., neg_class_5] * num_MMD_NCA_Groups]
        self.MMD_NCA_Classes = []

        for _ in range(num_MMD_NCA_Groups):
            np.random.shuffle(classes_all)
            classes = classes_all[:6]
            classes.insert(0, classes_all[0])
            
            self.MMD_NCA_Classes.append(classes)

            MMD_NCA_Group = []
            for item_class in classes:
                np.random.shuffle(available_datasets[item_class])
                MMD_NCA_Group.append(available_datasets[item_class][:25])

            self.MMD_NCA_Groups.append(MMD_NCA_Group)
        
        del available_datasets
        gc.collect()
        
    def __getitem__(self, index):
        group = self.MMD_NCA_Groups[index]
        classes = self.MMD_NCA_Classes[index]

        trial_idx = group[0][0][0]
        serial_start_idx = group[0][0][1]
        item = self.raw[classes[0]][trial_idx][serial_start_idx : serial_start_idx + config.input_length]
        for j in range(1, 25):
            trial_idx = group[0][j][0]
            serial_start_idx = group[0][j][1]
            item = np.concatenate((item, self.raw[classes[0]][trial_idx][serial_start_idx : serial_start_idx + config.input_length]), axis = 0)

        for i in range(1, 7):
            for j in range(25):
                trial_idx = group[i][j][0]
                serial_start_idx = group[i][j][1]
                item = np.concatenate((item, self.raw[classes[i]][trial_idx][serial_start_idx : serial_start_idx + config.input_length]), axis = 0)

        return item
        
    def __len__(self):
        return self.num_MMD_NCA_Groups

def train(model, train_loader, myloss, optimizer, epoch):
    model.train()
    for batch_idx, train_data in enumerate(train_loader):
        train_data = Variable(train_data).type(torch.cuda.DoubleTensor).squeeze().view(175, config.input_length, config.feature_size).permute(1,0,2)
        optimizer.zero_grad()
        # train_data.shape: [50, 175, 57], 50: squence length, 175: batch size, 57 input size
        output = model(train_data)
        loss = myloss(output)
        loss.backward()
        optimizer.step()
        print('Train Epoch: {} \tloss: {:.2f}'.format(epoch, 10000.*loss.data.cpu().numpy()))
        return loss

if __name__ == "__main__":
    #generate training data
    train_data = MMD_NCA_Dataset(config.processed_dir, config.num_MMD_NCA_Groups)
    train_loader = DataLoader(train_data, batch_size = 1, shuffle = True)

    start_epoch, model, optimizer = utils.load_model()
    epoch = start_epoch
    criterion = MMD_NCA_loss()
    loss_total = 0.

    loss_log = open(os.path.join(config.model_dir, "loss.log"), "a")

    while epoch <= config.num_epochs:
        loss = train(model, train_loader, criterion, optimizer, epoch)
        loss_total += loss.data.cpu().numpy()

        if epoch % config.show_loss_epoch == 0 and epoch != start_epoch:
            loss_log.write("epoch: {}, loss: {}\n".format(epoch, loss_total / config.show_loss_epoch))    
            loss_log.flush()
            print('loss mean after {} epochs: {}'.format(epoch, loss_total / config.show_loss_epoch))    
            loss_total = 0.
        
        if epoch % config.save_epoch == 0 and epoch != start_epoch:
            utils.save_model(epoch, model, optimizer)
        
        if epoch % config.shuffle_epoch == 0 and epoch != start_epoch:
            del train_data
            del train_loader
            gc.collect()
            train_data = MMD_NCA_Dataset(config.processed_dir, config.num_MMD_NCA_Groups)
            train_loader = DataLoader(train_data, batch_size = 1, shuffle = True)
        
        epoch += 1
