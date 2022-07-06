import json
import os
import sys

import config

if __name__ == "__main__":
    len = len(sys.argv)
    records = []
    out_filename = os.path.join(config.processed_dir, "unknown.json")
    for i in range(1, len):
        arg = sys.argv[i]
        if arg.isdigit():
            filename = os.path.join(config.record_dir, arg + ".json")
            f = open(filename)
            record = json.load(f)
            f.close()
            records.append(record)
        else:
            filename = os.path.join(config.processed_dir, arg + ".json")
            out_filename = filename
            if os.path.exists(filename):
                f = open(filename)
                existed_records = json.load(f)
                f.close()
                records += existed_records
    f = open(out_filename, "w+")
    json.dump(records, f)
    f.close()

    print("combined records written into {}".format(out_filename))
