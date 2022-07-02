model_dir = "./model"
dataset_dir = "./dataset"
train_data = "total.json"

html = "./dataset/raw/mocap.html"
xyz_dir = "./dataset/xyz"
meta_dir = "./dataset/meta"
processed_dir = "./dataset/processed"

train_classes = [
    ["wrench", 0.5],
    ["wash", 0.5],
    ["sweep", 0.5],
    ["laugh", 0.5],
    ["screw", 0.5],
    ["hammer", 0.5],
    ["climb", 0.5],
    ["coil", 0.5],
    ["walk", 0.5],
    ["run", 0.2],
]

train_length = 5
train_framerate = 10
train_frames = train_length * train_framerate

input_size = 19 * 3
learning_rate = 0.0001
num_epochs = 300000
save_epoch = 10000
shuffle_epoch = 300000
show_loss_epoch = 1000
num_MMD_NCA_Groups = 50000