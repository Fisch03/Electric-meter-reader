import serial
import time

s = serial.Serial('/dev/ttyACM0', 9600) # Put your Arduino Port here
s.open()
time.sleep(5)

s.write("test")
try:
    while True:
        response = s.readline()
        print(response)
except KeyboardInterrupt:
