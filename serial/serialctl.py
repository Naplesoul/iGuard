import serial
import time

import socket

portx="/dev/ttyACM0"
last_time = time.time()
dl = "@"

ser = serial.Serial(portx, 9600, timeout=1, parity=serial.PARITY_NONE)
if (ser.isOpen()):
    print("open success")
else:
    print("cannot open serial")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 40790))

while True:
    msg, _  = sock.recvfrom(1)
    cur_time = time.time()
    if (cur_time > last_time + 0.49):
        ser.write(" ".encode())
    last_time = cur_time
    ser.write(msg)