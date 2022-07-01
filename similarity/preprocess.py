import numbers
import os
import json
import threading

base_dir = "./dataset/xyz"
meta_dir = "./dataset/meta"
out_dir = "./dataset/processed"

joints = ['root', 'lowerback', 'upperback', 'thorax', 'lowerneck', 'upperneck', 'head',
          'rclavicle', 'rhumerus', 'rradius', 'rwrist', 'rhand', 'rfingers', 'rthumb',
          'lclavicle', 'lhumerus', 'lradius', 'lwrist', 'lhand', 'lfingers', 'lthumb',
          'rfemur', 'rtibia', 'rfoot', 'rtoes', 'lfemur', 'ltibia', 'lfoot', 'ltoes', 'lhipjoint', 'rhipjoint']

used_joints = ['head', 'lowerneck', 'lowerback', 'root', 'thorax',
               'rclavicle', 'rhumerus', 'rwrist', 'rhand',
               'lclavicle', 'lhumerus', 'lwrist', 'lhand',
               'rhipjoint', 'rfemur', 'rtibia',
               'lhipjoint', 'lfemur', 'ltibia']

output_framerate = 15

classes = [
    "wrench",
    "wash",
    "sweep",
    "laugh",
    "screw"
]

def process_serial(json, input_framerate, start_sec, end_sec):
    serial = []
    sample_rate = input_framerate / output_framerate

    if start_sec < 0:
        start = 0
        end = len(json[joints[0]])
    else:
        start = int(start_sec * input_framerate)
        end = int(end_sec * input_framerate)
        if end > len(json[joints[0]]):
            end = len(json[joints[0]])
    
    for i in range(start, end):
        if i % sample_rate != 0:
            continue

        vector = []
        for joint in used_joints:
            vector.append([j * 10 for j in json[joint][i]])
        serial.append(vector)
    return serial

def process_class(classname, trails):
    output = []
    for trial in trails:
        split = trial[0].split("-")
        folder = split[0]
        path = os.path.join(base_dir, folder, "{}_{}.json".format(folder, split[1]))
        f = open(path, "r")
        serial = json.load(f)
        f.close()

        if isinstance(trial[2], (int, float)):
            output.append(process_serial(serial, trial[1], trial[2], trial[3]))
        else:
            output.append(process_serial(serial, trial[1], -1, -1))

    out_path = os.path.join(out_dir, classname + ".json")
    o = open(out_path, "w+")
    json.dump(output, o)
    o.close()

if __name__ == "__main__":
    os.makedirs(out_dir, exist_ok=True)
    
    threads = []
    for classname in classes:
        f = open(os.path.join(meta_dir, classname + ".json"))
        trails = json.load(f)
        f.close()
        thread = threading.Thread(target=process_class, args=[classname, trails])
        thread.start()
        threads.append(thread)
    
    for thread in threads:
        thread.join()
