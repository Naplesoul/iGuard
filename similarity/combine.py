import os
import json

import config

if __name__ == "__main__":
    motions = os.listdir(config.record_dir)
    for m in motions:
        dir = os.path.join(config.record_dir, m)
        files = os.listdir(dir)
        records = []

        for f in files:
            file = open(os.path.join(dir, f))
            records.append(json.load(file))
            file.close()
        
        out_filename = os.path.join(config.processed_dir, m + ".json")
        if os.path.exists(out_filename):
            print(f"combine {dir} with {out_filename}")
            f = open(out_filename)
            existed_records = json.load(f)
            f.close()
            records += existed_records
        else:
            print(f"combine {dir} into {out_filename}")
        
        f = open(out_filename, "w+")
        json.dump(records, f)
        f.close()
