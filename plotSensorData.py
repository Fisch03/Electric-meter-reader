import serial #Serial Communication

#Timestamps
import time
import datetime as dt

 #Plotting
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.style as style

s = serial.Serial(port='/dev/ttyACM0', baudrate=9600, timeout=61) #Put your Arduino Port here

currentw = 0.0
lastw = 0.0

try:
    s.open() #Open Serial Connection
except serial.serialutil.SerialException:
    print("[" + dt.datetime.now().strftime('%X') + "] Port is already open, continuing anyways")

#Configure the Plot
style.use('ggplot')
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

#Wait 10s to let the Arduino Restart
print("[" + dt.datetime.now().strftime('%X') + "] Waiting for Arduino to restart...")
time.sleep(10)

def animate(i, xs, ys):
    if(currentw != 0):
        if(currentw != lastw):
            global lastw

            print("["+ dt.datetime.now().strftime('%X') + "] " + str(currentw) + " Watt") #Print out the Data
            xs.append(dt.datetime.now()) #Append a Timestamp to the X Axis
            ys.append(currentw) #Append it to the y Axis
            lastw = currentw
    else:
        print("["+ dt.datetime.now().strftime('%X') +"] Data is still calculating, skipping")

    ax.clear()
    ax.plot(xs, ys)

    #Label the Plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title("Power Usage in W over Time")
    plt.ylabel("Watt")
    plt.xlabel("Time")

try:
    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=1000) #Change Update Speed here (in ms)
    plt.show(0)
    while True:
        try:
            currentw = float(s.readline()) #Read the new Serial Data
            plt.pause(0.0001) #Pause te Plot to let the Program execute
        except ValueError:
            print("[" + dt.datetime.now().strftime('%X') + "] Failed to fetch Data")
except KeyboardInterrupt:
    s.close()
