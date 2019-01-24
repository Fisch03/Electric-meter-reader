Electric Meter Reader
===

A simple Arduino Program, which reads Electric meters using 2 tracking Sensors.
You can retrieve the Data either by using a Webinterface, provided by the Arduino Ethernet Shield, a Raspberry PI running a Python Script, which Plots the past usage or by using [Emoncms](https://emoncms.org/), a free web app for data visualization.


Installation
---

**You will need:**
- Arduino Mega / Due
- 2x Tracking Sensor

**For the Webinterface:**
- Ethernet Cable with working Connection
- Arduino Ethernet Shield

**For the Python Plotting:**
- Raspberry PI
- WiFi or Ethernet Connection (Only at Installation)

### Preparing the Arduino File
Open up the Electric_Meter.ino File. Now you have to fill in some Values:
- "#define MeterNumber" Input the Number of Meters you want to read. you will need 2 Analog Outputs and corresponding Sensors per Meter.
- "int sensorpins" For each Meter, input the two Analog Arduino Pins you will use.
- "float turnsperkwh" For each Meter, input the Number of turns that equal 1 KWh. You will usually find this number somewhere on your Meter.

### Connecting the Sensors
Connect your sensors to Ground and Power. Plug the Analog Output of your Sensors into the Arduino Inputs you chose in the Electric_Meter.ino File (see "Preparing the Arduino File"). Use a Piece of Cardboard (or similar) to secure both Sensors in Front of the spinning Wheel on your Electric Meter.

### Setting up the Webinterface
Place the Ethernet Shield on the Arduino. To find a fitting IP Adress for your Arduino, connect it to your Network via the Ethernet Cable, and Connect it to your PC. In the Arduino IDE, go to File > Examples > Ethernet > DhcpAddressPrinter and Upload it. When you Open the Serial Monitor (Tools > Serial Monitor), you should see an IP Adress. Note it for later.
Open the Electric_Meter.ino File and find the Line that says "Your Arduino IP Adress". Input the Address from earlier (Make sure to use commas instead of periods). Now you can Flash your Arduino. Open up the IP Address in a Browser and you should see the Current Readings.

### Setting up the Python Plotting
To do this, you will need a few Python Libraries installed. Open up your Terminal and type
```
sudo python -m pip install -U matplotlib
sudo python -m pip install -U pyserial
```
This should install matplotlib and pyserial on your Pi.
Next, you need to find the Port, your Arduino is connected to. Open up the Terminal again and type
```
ls /dev/tty*
```
Now Plug in your Arduino and issue the Command again. There should be a new entry in the List. Note the name again. Open up the SerialCommunicationTest.py file and Find the line that says "Put your Arduino Port here". Input your Port name. Execute the Python Script and check if there is any Data incoming. If it works, you are nearly done. Open the plotSensorData.py File and replace the Port again. You will also have to input the Number of Meters you are using again ("MeterNumber"). You are now ready to plot your live Usage!

### Sending your Data to Emoncms
If your Raspberry Pi has constant access to the Internet, you can also send your Data to Emoncms. Open the "plotSensorData.py" Filem and change the "EmoncmsUsage" Variable to True. You will then have to install another Python package. Open the Terminal and type
```
sudo python -m pip install -U requests
```
Log into your Emoncms Account or create a new one. Go to Setup > API and copy your Read & Write key into the "secretsexample.py" File. **Do not share this key as it will grant Full access to your Emoncms Account** Change the Name of the File into "secrets.py". If you run the Script, you should find the Outputs of your Meter in Emoncms under Setup > Inputs. Configuring a Dashboard is beyond the Scope of this Tutorial, but I recommend you [this](https://powerforum.co.za/topic/1245-emoncms-startup-tutorial/) Site to get started.


### Notes
- The Meter has to do at least 1 Full turn to calculate the usage. Before that, the usage will show up as 0. This has to do with the way the Calculating is done, and there is no Fix for that.
- To exit the Python Script, close down the Plot Window (Alt + F4), then hit Ctrl + C in the Terminal.
- Starting the Script may reset the Arduino because a new Serial connection is made. If this happens, you will lose all Prerecorded Turns.
- If you want to have a visual Indicator for the Disc on your electric Meter, you can Connect two LEDs to the Arduino Outputs you specified in the Electric_Meter.ino File (see "int ledpins"). These will light up if the Sensors detect the Color on the Disk.

### Troubleshooting
- The Arduino samples a default Value when started. If the Color on the Disk is in front of it while doing so, the Arduino might fail to read the Electric Meter. Try resetting it by Pressing its reset Button when the Color is **not** in front of it.
- Depending on the Room you install the System in, there may be too much Noise. You can tweak the sens1 and sens2 values in the Electric_Meter.ino File, to counteract this.
- If you notice a lot of Noise in the Sensor readings, try Increasing the "smoothing" constant in the Electric_Meter.ino File. Try to keep this Value as low as possible, because it will slow down the response Time of your Arduino.
- The Python Script has a maximum Value to prevent Serial Communication Errors. If your Power usage exceeds 7500 Watts, you should change it to something higher.
