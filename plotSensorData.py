import serial

import time
import datetime as dt

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.style as style

s = serial.Serial('/dev/ttyACM0', 9600) #Put your Arduino Port here

try:
    s.open()
except serial.serialutil.SerialException:
    print("Port is already open, continuing")

style.use('ggplot')
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

time.sleep(5)

def animate(i, xs, ys):
    s.readline()
    s.readline()
    try:
        currentw = float(s.readline())
        print(currentw)
        xs.append(dt.datetime.now().strftime('%H:%M:%S'))
        ys.append(currentw)

    except ValueError:
        print("Failed to get Data")

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
except KeyboardInterrupt:
    s.close()
