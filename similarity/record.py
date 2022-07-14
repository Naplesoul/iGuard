import os
import sys
import json
import socket

import config
import server

# frame_num = config.ideal_input_length_sec * config.model_framerate

if __name__ == "__main__":
    if not os.path.exists(config.record_dir):
        os.makedirs(config.record_dir)

    while 1:
        frame_num = int(input("> frame num? "))
        filename = input(">") + ".json"
        print("start recording " + str(frame_num) + " frames")

        result = []
        recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        recv.bind(("0.0.0.0", config.listen_port))
        print("-" * frame_num, end="")
        sys.stdout.flush()
        for i in range(frame_num):
            result.append(server.get_skeleton(recv))
            print("\b" * frame_num + "#" * (i + 1) + "-" * (frame_num - i - 1), end="")
            sys.stdout.flush()
        
        f = open(os.path.join(config.record_dir, filename), "w+")
        json.dump(result, f)
        f.close()
        
        print("\nsave result to " + filename)
