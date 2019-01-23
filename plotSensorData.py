import serial #Serial Communication

#Timestamps
import time
import datetime as dt

 #Plotting
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import matplotlib.style as style

s = serial.Serial(port='/dev/ttyACM0', baudrate=9600) #Put your Arduino Port here

MeterNumber = 2
EmoncmsUsage = False

if (EmoncmsUsage):
    from secrets import emoncmskey
    import requests

currentw = [0.00, 0.00]
lastw = [0.00, 0.00]

try:
    s.open() #Open Serial Connection
except serial.serialutil.SerialException:
    print("[" + dt.datetime.now().strftime('%X') + "] Port is already open, continuing anyways")

#Configure the Plot
style.use('ggplot')
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = [[],[]]
ys = [[],[]]

#Wait 10s to let the Arduino Restart
print("[" + dt.datetime.now().strftime('%X') + "] Waiting for Arduino to restart...")
time.sleep(10)
print("[" + dt.datetime.now().strftime('%X') + "] Started Program")


def animate(i, xs, ys):
    ax.clear()
    for s in range(MeterNumber):
        if(currentw[s] != 0):
            if(currentw[s] != lastw[s]):
                global lastw
                xs[s].append(dt.datetime.now()) #Append a Timestamp to the X Axis

                print("["+ dt.datetime.now().strftime('%X') + "] " + str(currentw[s]) + " Watt") #Print out the Data
                ys[s].append(currentw[s]) #Append it to the y Axis
                lastw[s] = currentw[s]
        else:
            print("["+ dt.datetime.now().strftime('%X') +"] Data is still calculating, skipping")

        print(ys[s])
        ax.plot(xs[s], ys[s])

    #Lable the Plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title("Power Usage in W over Time")
    plt.ylabel("Watt")
    plt.xlabel("Time")

    if(EmoncmsUsage):
        sensorjson = "{"
        for sens in range(MeterNumber-1):
            sensorjson += '"Meter' + str(sens) + '":' + str(currentw[sens]) + ', '
        sensorjson += '"Meter' + str(MeterNumber-1) + '":' + str(currentw[MeterNumber-1]) + '}'
        payload = {'fulljson':sensorjson, 'apikey':emoncmskey, 'node':'Electric_Meter'}
        requests.post("https://emoncms.org/input/post", params=payload)

try:
    ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=1000)
#    mng = plt.get_current_fig_manager()
#    mng.full_screen_toggle() #Put the window in Fullscreen Mode
    plt.show(0)
    while True:
        try:
            serdata = s.readline() #Read the new Serial Data
            serdata = serdata.split("|")
            for w in range(len(serdata)-1):
                currentw[w] = float(serdata[w])

            plt.pause(0.0001) #Pause the Plot to let the Program execute
        except ValueError:
            print("[" + dt.datetime.now().strftime('%X') + "] Failed to fetch Data")
except KeyboardInterrupt:
    s.close()
