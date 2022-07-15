import serial
import serial.tools.list_ports

import socket

portx="/dev/ttyACM0"

ser = serial.Serial(portx, 9600, timeout=1, parity=serial.PARITY_NONE)
if (ser.isOpen()):
    print("open success")
else:
    print("cannot open serial")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 40790))

while True:
    msg, _  = sock.recvfrom(1)
    ser.write(msg)