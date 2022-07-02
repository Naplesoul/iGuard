import os
import json
import threading

import config

base_dir = config.xyz_dir
meta_dir = config.meta_dir
out_dir = config.processed_dir

joints = ['root', 'lowerback', 'upperback', 'thorax', 'lowerneck', 'upperneck', 'head',
          'rclavicle', 'rhumerus', 'rradius', 'rwrist', 'rhand', 'rfingers', 'rthumb',
          'lclavicle', 'lhumerus', 'lradius', 'lwrist', 'lhand', 'lfingers', 'lthumb',
          'rfemur', 'rtibia', 'rfoot', 'rtoes', 'lfemur', 'ltibia', 'lfoot', 'ltoes', 'lhipjoint', 'rhipjoint']

used_joints = ['head', 'upperneck', 'lowerback', 'root', 'lowerneck',
               'rclavicle', 'rhumerus', 'rwrist', 'rhand',
               'lclavicle', 'lhumerus', 'lwrist', 'lhand',
               'rhipjoint', 'rfemur', 'rtibia',
               'lhipjoint', 'lfemur', 'ltibia']

classes = config.train_classes

def process_serial(json, input_framerate, start_sec, end_sec, scale):
    serial = []
    sample_rate = int(input_framerate / config.train_framerate)

    if start_sec < 0:
        start = 0
        end = len(json[joints[0]])
    else:
        start = int(start_sec * input_framerate)
        end = int(end_sec * input_framerate)
        if end > len(json[joints[0]]):
            end = len(json[joints[0]])
    
    if end - start < config.train_length * input_framerate:
        if end - start < scale * config.train_length * input_framerate:
            print("dataset not sufficient for training: {} / {}, skipping...".format(end - start, config.train_length * input_framerate))
            return []
        
        required_frames = (config.train_length + 1) * config.train_framerate
        sample_rate = int((end - start) / required_frames)
        print("dataset not sufficient for training: {} / {}, change sample rate to {}".format(end - start, config.train_length * input_framerate, sample_rate))
    else:
        print("dataset ok")

    for i in range(start, end):
        if i % sample_rate != 0:
            continue

        vector = []
        for joint in used_joints:
            vector.append([j * 10 for j in json[joint][i]])
        serial.append(vector)
    return serial

def process_class(classname, trails, sclae):
    outputs = []
    for trial in trails:
        split = trial[0].split("-")
        folder = split[0]
        path = os.path.join(base_dir, folder, "{}_{}.json".format(folder, split[1]))
        f = open(path, "r")
        serial = json.load(f)
        f.close()

        if isinstance(trial[2], (int, float)):
            output = process_serial(serial, trial[1], trial[2], trial[3], scale)
        else:
            output = process_serial(serial, trial[1], -1, -1, scale)
            
        if len(output) > 0:
            outputs.append(output)

    out_path = os.path.join(out_dir, classname + ".json")
    o = open(out_path, "w+")
    json.dump(outputs, o)
    o.close()

if __name__ == "__main__":
    os.makedirs(out_dir, exist_ok=True)
    
    threads = []
    for class_vec in classes:
        classname = class_vec[0]
        scale = class_vec[1]

        f = open(os.path.join(meta_dir, classname + ".json"))
        trails = json.load(f)
        f.close()
        thread = threading.Thread(target=process_class, args=[classname, trails, scale])
        thread.start()
        threads.append(thread)
    
    for thread in threads:
        thread.join()
