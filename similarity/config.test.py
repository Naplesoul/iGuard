model_dir = "./model"

html = "./dataset/raw/mocap.html"
xyz_dir = "./dataset/xyz"
meta_dir = "./dataset/meta"
record_dir = "./dataset/record"
processed_dir = "./dataset/processed"

gui_ip = "192.168.0.111"
gui_port = 50002
gui_sim_port = 50003
listen_port = 50001

use_cuda = True

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
    ["cry", 0.5],
    ["squat", 0.5],
    ["wave", 0.5],
    ["swim", 0.5]
]

input_length_sec = 5
model_framerate = 10
joint_size = 13
feature_size = (joint_size - 1) * 3
learning_rate = 0.0001
num_epochs = 50000
save_epoch = 1000
shuffle_epoch = 10000
show_loss_epoch = 100
num_MMD_NCA_Groups = 10000

ideal_input_length_sec = input_length_sec + 1
input_length = input_length_sec * model_framerate