# import bluetooth


# nearby_devices = bluetooth.discover_devices(lookup_names=True)
# print("Found {} devices.".format(len(nearby_devices)))

# for addr, name in nearby_devices:
#     print("  {} - {}".format(addr, name))


# deviceName = "HC-05"
# deviceAddr = "98:D3:31:F6:21:FC"

# port = 1


# uuid = "00001101-0000-1000-8000-00805F9B34FB"
# service_matches = bluetooth.find_service(uuid=uuid, address=deviceAddr)

# Create the client socket
# sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
# sock.connect((deviceAddr, port))

# while True:
#     received = sock.recv();
#     print (received)


from telnetlib import NOP
from turtle import delay
from matplotlib.animation import FuncAnimation
import serial
import time
import struct
import numpy as np
import matplotlib.pyplot as plt
import concurrent.futures
import threading

import socket

HOST = "192.168.1.174"  # Standard loopback interface address (localhost)
PORT = 65432             # Port to listen on (non-privileged ports are > 1023)

def main():
    print ("main")
    # open a tcp server
    # esp8266 can connect to the server
    # esp sends data
    # print out data from a client
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            while True:
                data = conn.recv(1024)
                print (data)

if __name__ == "__main__":
    main()
