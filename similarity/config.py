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

input_length_sec = 5
model_framerate = 10
feature_size = 19 * 3
learning_rate = 0.0001
num_epochs = 50000
save_epoch = 1000
shuffle_epoch = 2000
show_loss_epoch = 100
num_MMD_NCA_Groups = 3000

input_length = input_length_sec * model_framerate