#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02}; //Your Arduino MAC Adress
IPAddress ip(192,168,44,193); //Your Arduino IP Address

EthernetServer server(80); //Default HTTP Port

#define MeterNumber 2 //The number of Meters to read

//Arduino Pins for Sensors and LEDs
int sensorpins[MeterNumber*2] = {A8,A9, A10,A11};
int ledpins[MeterNumber*2] = {2,3, 4,5};

//Sensitivity for each Sensor
int sens[MeterNumber*2] = {100,65, 100,100};

float turnsperkwh[MeterNumber] = {73.00, 73.00}; //Put your Values here

#define smoothing 16 //Smoothing Value (Values over 64 slow down the Arduino significantly)

float whperturn[MeterNumber];

long turnstarttime[MeterNumber];
float currentw[MeterNumber];

int default1[MeterNumber], default2[MeterNumber];

int s1afiltered[MeterNumber], s2afiltered[MeterNumber];

bool s1[MeterNumber], s2[MeterNumber], lasts1[MeterNumber], lasts2[MeterNumber];

float time1[MeterNumber], time2[MeterNumber], time1start[MeterNumber], time2start[MeterNumber];
bool time1started[MeterNumber], time2started[MeterNumber], time1set[MeterNumber], time2set[MeterNumber];

int count[MeterNumber];

void setup() {
  //Start the Serial connection
  Serial.begin(9600);

  //Start the HTTP Server
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server ist Online, IP:");
  Serial.println(Ethernet.localIP());

  for (int currentmeter = 0; currentmeter < MeterNumber; currentmeter++) {
    whperturn[currentmeter] = (1.00/turnsperkwh[currentmeter])*1000.00;

    //Read in the Default values for all Sensors
    default1[currentmeter] = analogRead(sensorpins[currentmeter]);
    default2[currentmeter] = analogRead(sensorpins[currentmeter+1]);
    lasts1[currentmeter] = analogRead(sensorpins[currentmeter]) - default1[currentmeter] > sens[currentmeter];
    lasts2[currentmeter] = analogRead(sensorpins[currentmeter+1]) - default2[currentmeter] > sens[currentmeter+1];
  }
}

void loop() {
  for(int currentmeter = 0; currentmeter < MeterNumber; currentmeter++){
    updatesensors(currentmeter);
    Serial.print(currentw[currentmeter]);
    Serial.print("|");
  }
  Serial.println();

  EthernetClient client = server.available();

  if (client) { //Check if a client is connected
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && currentLineIsBlank) {
          //HTTP Header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 1"); //Refresh the page each Second
          client.println();

          //Show all Important Information
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          for (int currentmeter = 0; currentmeter < MeterNumber; currentmeter++) {
            client.print("Meter Number: ");
            client.print(currentmeter+1);
            client.print(" || Current Power usage: ");
            client.print(currentw[currentmeter]);
            client.print(" W || Turns since beginning: ");
            client.print(count[currentmeter]);
            client.print(" || Default Values for Sensors: ");
            client.print(default1[currentmeter]);
            client.print(", ");
            client.print(default2[currentmeter]);
            client.print(" || Current Values for Sensors: ");
            client.print(analogRead(sensorpins[currentmeter]));
            client.print(", ");
            client.print(analogRead(sensorpins[currentmeter+1]));
            client.print("<br>");
          }
          client.println("</html>");
          break;
        }

        //Wait for the end of the HTTP Request
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void updatesensors(int currentmeter) {
  //Read the analog Value multiple Times, then take the average to prevent Noise
  for (int i = 0; i < smoothing; ++i) {
    s1afiltered[currentmeter] += analogRead(sensorpins[currentmeter]);
    s2afiltered[currentmeter] += analogRead(sensorpins[currentmeter+1]);
    delay(1);
  }
  s1afiltered[currentmeter] = s1afiltered[currentmeter] / smoothing;
  s2afiltered[currentmeter] = s2afiltered[currentmeter] / smoothing;

  //Check if the Sensors are Triggered
  s1[currentmeter] = s1afiltered[currentmeter] - default1[currentmeter] > sens[currentmeter];
  s2[currentmeter] = s2afiltered[currentmeter] - default2[currentmeter] > sens[currentmeter+1];

  //Update the LEDs
  if(s1[currentmeter]) {
    digitalWrite(ledpins[currentmeter], HIGH);
  } else {
    digitalWrite(ledpins[currentmeter], LOW);
  }
  if(s2[currentmeter]) {
    digitalWrite(ledpins[currentmeter+1], HIGH);
  } else {
    digitalWrite(ledpins[currentmeter+1], LOW);
  }

  /**
  if (s1 && !lasts1) {
    if(s2 != s1) {
      count++;
    } else {
      count--;
    }
  }
  **/

  if (s1[currentmeter] && !lasts1[currentmeter]) { //Positive Edge of s1
    if(!time1started[currentmeter]) { //Start measuring t1 if it isn't being measured already
      startt1(currentmeter);
    } else if (time2started && time1set) { //If both Times are stopped, we update our Count, depending on their lengths
      stopt2(currentmeter);

      time1set[currentmeter] = false;
      time2set[currentmeter] = false;
      time1started[currentmeter] = false;
      time2started[currentmeter] = false;

      if (time1[currentmeter] < time2[currentmeter]) {
        count[currentmeter]++;
      } else {
        count[currentmeter]--;
      }

      startt1(currentmeter);

      if(turnstarttime[currentmeter] != 0) {
        currentw[currentmeter] = (float)whperturn[currentmeter]/(((float)millis()-(float)turnstarttime[currentmeter])/3600000.00);
      }

      turnstarttime[currentmeter] = (float)millis();
    }
  }
  if (s2[currentmeter] && !lasts2[currentmeter]) { //Positive Edge of s2
    if(!time2started[currentmeter] && time1started[currentmeter]) { //Start measuring t2 if it isn#t being measured already
      startt2(currentmeter);
      stopt1(currentmeter);
    }
  }

  //Update lasts1 and lasts2
  lasts1[currentmeter] = s1[currentmeter];
  lasts2[currentmeter] = s2[currentmeter];
}

void startt1(int currentmeter) { //Start measuring t1
  time1start[currentmeter] = millis();
  time1started[currentmeter] = true;
}

void stopt1(int currentmeter) { //Stop measuring t1
  time1[currentmeter] = millis() - time1start[currentmeter];
  time1set[currentmeter] = true;
}

void startt2(int currentmeter) { //Start measuring t2
  time2start[currentmeter] = millis();
  time2started[currentmeter] = true;
}

void stopt2(int currentmeter) { //Stop measuring t2
  time2[currentmeter] = millis() - time2start[currentmeter];
  time2set[currentmeter] = true;
}
