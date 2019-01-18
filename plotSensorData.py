import serial

import time
import datetime as dt

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.style as style

s = serial.Serial(port='/dev/ttyACM0', baudrate=9600, timeout=61) #Put your Arduino Port here

currentw = 0.0

try:
    s.open()
except serial.serialutil.SerialException:
    print("[" + dt.datetime.now().strftime('%X') + "] Port is already open, continuing anyways")

style.use('ggplot')
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

print("[" + dt.datetime.now().strftime('%X') + "] Waiting for Arduino to restart...")
time.sleep(10)
print("[" + dt.datetime.now().strftime('%X') + "] Syncing with Arduino, this might take up to a minute")

def animate(i, xs, ys):
    if(currentw != 0):
        print("["+ dt.datetime.now().strftime('%X') + "] " + str(currentw) + " Watt")
        xs.append(dt.datetime.now().strftime('%X'))
        ys.append(currentw)
    else:
        print("["+ dt.datetime.now().strftime('%X') +"] Data is still calculating, skipping")

    xs = xs[-60:]
    ys = ys[-60:]

    ax.clear()
    ax.plot(xs, ys)

    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title("Power Usage in W over Time")
    plt.ylabel("Watt")
    plt.xlabel("Time")

try:
    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=60000)
    plt.show()
    while True:
        try:
            currentw = float(s.readline())
        except ValueError:
            print("[" + dt.datetime.now().strftime('%X') + "] Failed to fetch Data")
except KeyboardInterrupt:
    s.close()
