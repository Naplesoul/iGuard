import os
import sys
import json
import socket

import config

frame_num = config.ideal_input_length_sec * config.model_framerate

if __name__ == "__main__":
    while 1:
        filename = input(">") + ".json"
        print("start recording " + str(frame_num) + " frames")

        result = []
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("0.0.0.0", config.server_port))
        print("-" * frame_num, end="")
        sys.stdout.flush()
        for i in range(frame_num):
            bytes, addr = s.recvfrom(10240)
            node_info = json.loads(bytes.decode())
            frame = []
            for j in range(25):
                if j in [0, 10, 11, 16, 20, 24]:
                    continue
                frame.append([node_info["nodes"][j]["x"], node_info["nodes"][j]["y"], node_info["nodes"][j]["z"]])
            result.append(frame)
            print("\b" * frame_num + "#" * (i + 1) + "-" * (frame_num - i - 1), end="")
            sys.stdout.flush()
        
        f = open(os.path.join(config.record_dir, filename), "w+")
        json.dump(result, f)
        f.close()
        
        print("\nsave result to " + filename)
