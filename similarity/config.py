from random import shuffle


model_dir = "./model"
dataset_dir = "./dataset"
train_data = "total.json"

learning_rate = 0.0001
num_epochs = 300000
save_epoch = 2000
show_loss_epoch = 1000
num_MMD_NCA_Groups = 50000