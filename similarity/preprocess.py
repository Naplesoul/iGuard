import os
import json
import threading

base_dir = "./dataset/xyz"
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

classes = {
    "walk": [
        '02-01',
        '02-02',
        '03-01',
        '03-02',
        '03-03',
        '03-04',
        '05-01',
        '06-01',
        '07-01',
        '07-02',
        '07-03',
        '07-04',
        '07-05',
        '07-06',
        '07-07',
        '07-08',
        '07-09',
        '07-10',
        '07-11',
        '07-12',
    ]
}

def process_serial(json):
    serial = []
    l = len(json[joints[0]])
    for i in range(l):
        vector = []
        for joint in used_joints:
            vector.append([j * 10 for j in json[joint][i]])
        serial.append(vector)
    return serial

def process_class(classname):
    output = []
    for trial in classes[classname]:
        split = trial.split("-")
        folder = split[0]
        path = os.path.join(base_dir, folder, "{}_{}.json".format(folder, split[1]))
        f = open(path, "r")
        serial = json.load(f)
        f.close()

        output.append(process_serial(serial))

    out_path = os.path.join(out_dir, classname + ".json")
    o = open(out_path, "w+")
    json.dump({ key: output }, o)
    o.close()

if __name__ == "__main__":
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    
    threads = []
    for key in classes:
        thread = threading.Thread(target=process_class, args=[key])
        thread.start()
        threads.append(thread)
    
    for thread in threads:
        thread.join()
