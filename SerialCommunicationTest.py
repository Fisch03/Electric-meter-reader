#Test your Serial Communication with this Python Script

import serial
import time

try:
    s = serial.Serial('/dev/ttyACM0', 9600) # Put your Arduino Port here
    s.open()
except serial.serialutil.SerialException:
    print("Port already open, continuing anyways")

time.sleep(5)

try:
    while True:
        response = float(s.readline())
        print(response)
except KeyboardInterrupt:
    s.close()
