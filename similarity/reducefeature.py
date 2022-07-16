import os
import json

import config

if __name__ == "__main__":
    motions = os.listdir(config.processed_dir)
    for m in motions:
        dir = os.path.join(config.processed_dir, m)
        f = open(dir)
        records = json.load(f)
        f.close()

        for i in range(len(records)):
            for j in range(len(records[i])):
                new_record = []
                anchor = records[i][j][4]
                for k in range(4):
                    new_record.append([records[i][j][k][0] - anchor[0], records[i][j][k][1] - anchor[1], records[i][j][k][2] - anchor[2]])
                for k in range(5, config.joint_size):
                    new_record.append([records[i][j][k][0] - anchor[0], records[i][j][k][1] - anchor[1], records[i][j][k][2] - anchor[2]])
                records[i][j] = new_record
        
        f = open(dir, "w+")
        json.dump(records, f)
        f.close()
        print(f"{dir} processed")