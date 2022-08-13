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

port = "COM10"
br = 921600

bluetooth = serial.Serial(port, br)
print ("Connected")

sendData ='hello\r\n'.encode('utf-8')

condition = threading.Condition()
arr = []

closeThread = False

def getBtData():
    global arr
    while True:
        if (closeThread):
            break
        #data = bluetooth.read_until()
        data = bluetooth.read(3072)
        print ("len: ", end='')
        print (len(data))
        x = struct.unpack('768f', data)
        condition.acquire()
        print ("Bt aquired")
        arr = np.round(np.reshape(np.asarray(x), (24, 32)), 1)
        condition.notify()
        print ("Bt notified")
        condition.release()

def main():
    global closeThread
    global arr

    plt.ion()
    # arr = np.zeros(shape=(24,32), dtype=np.float32)
    # img = plt.imshow(arr, cmap='jet')

    executor = concurrent.futures.ThreadPoolExecutor()
    f1 = executor.submit(getBtData)

    condition.acquire()
    print ("img aquired")
    condition.wait()
    print ("img waited")
    
    img = plt.imshow(arr, cmap='jet')
   
    try:
        while True:
            condition.acquire()
            print ("img aquired")
            condition.wait()
            print ("img waited")
            img.set_data(arr)
            condition.release()
            plt.pause(0.1)
            print ("img released")
        plt.ioff()
        plt.show()
    except KeyboardInterrupt:
        closeThread = True



# str = data.decode("utf-8")
# print ("Received: " + str[:len(str)-1])
# bluetooth.write(sendData)
# time.sleep(1)

if __name__ == "__main__":
    main()
