import serial
import time

import socket
import threading


def receiveMessage():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", 40790))
    while True:
        msg, _  = sock.recvfrom(2)
        msg_str = msg.decode()
        print("recv: " + msg_str)
        lock.acquire()
        if msg_str[0] == "<":
            dl_list[msg_str[1]] = 1
        elif msg_str[0] == ">":
            dl_list[msg_str[1]] = 0
        elif msg_str[0] == " ":
            for level in dl_list:
                dl_list[level] = 0
        lock.release()



portx="/dev/ttyACM0"
dl_list = {"F": 0, "E": 0, "D": 0, "C": 0, "B": 0, "A": 0}
lock = threading.Lock()

ser = serial.Serial(portx, 9600, timeout=1, parity=serial.PARITY_NONE)
if (ser.isOpen()):
    print("open success")
else:
    print("cannot open serial")
thread1 = threading.Thread(name='t1',target= receiveMessage, args=())
thread1.start()

last_dl = " "
while True:
    time.sleep(0.1)
    flag = False
    lock.acquire()
    for level in dl_list:
        if dl_list[level]:
            if level != last_dl:
                ser.write(level.encode())
                last_dl = level
            print("send: "+ level)
            flag = True
            break
    if (not flag) and (last_dl != " "):
        print("send:  ")
        ser.write(" ".encode())
        last_dl = " "
    lock.release()
