default#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02}; //Your Arduino MAC Adress
IPAddress ip(192,168,44,166); //Your Arduino IP Address

EthernetServer server(80); //Default HTTP Port

//Arduino Pins for Sensors and LEDs
#define s1a A8
#define s2a A9
#define led1 2
#define led2 3

//Sensitivity for each Sensor
#define sens1 150
#define sens2 100

#define smoothing 16 //Smoothing Value (Values over 64 slow down the Arduino significantly)
#define turnsperkwh 73.00 //Put your Value here

float whperturn = (1.00/turnsperkwh)*1000.00; //Calculate the Wh for one Turn
long turnstarttime;
float currentw;

int default1, default2;

int s1afiltered, s2afiltered;

bool s1, s2, lasts1, lasts2;

float time1, time2, time1start, time2start;
bool time1started, time2started, time1set, time2set;

int count = 0;
int diff;

void setup() {
  //Start the Serial connection
  Serial.begin(9600);

  //Start the HTTP Server
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server ist Online, IP:");
  Serial.println(Ethernet.localIP());

  //Read in the Default values for both Sensors
  default1 = analogRead(s1a);
  default2 = analogRead(s2a);
  Serial.println(default1);
  Serial.println(default2);
  lasts1 = analogRead(s1a) - default1 > sens1;
  lasts2 = analogRead(s2a) - default2 > sens2;
}

void loop() {
  updatesensors();

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
          client.print("Current Power usage: ");
          client.print(currentw);
          client.print(" W || Turns since beginning: ");
          client.print(count);
          client.print(" || Default Values for Sensors: ");
          client.print(default1);
          client.print(", ");
          client.print(default2);
          client.print(" || Current Values for Sensors: ");
          client.print(analogRead(s1a));
          client.print(", ");
          client.print(analogRead(s2a));
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

void updatesensors() {
  //Read the analog Value multiple Times, then take the average to prevent Noise
  for (int i = 0; i < smoothing; ++i) {
    s1afiltered += analogRead(s1a);
    s2afiltered += analogRead(s2a);
    delay(1);
  }
  s1afiltered = s1afiltered / smoothing;
  s2afiltered = s2afiltered / smoothing;

  //Check if the Sensors are Triggered
  s1 = s1afiltered - default1 > sens1;
  s2 = s2afiltered - standard2 > sens2;

  //Update the LEDs
  if(s1) {
    digitalWrite(led1, HIGH);
  } else {
    digitalWrite(led1, LOW);
  }
  if(s2) {
    digitalWrite(led2, HIGH);
  } else {
    digitalWrite(led2, LOW);
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

  if (s1 && !lasts1) { //Positive Edge of s1
    if(!time1started) { //Start measuring t1 if it isn't being measured already
      startt1();
    } else if (time2started && time1set) { //If both Times are stopped, we update our Count, depending on their lengths
      stopt2();

      time1set = false;
      time2set = false;
      time1started = false;
      time2started = false;

      if (time1 < time2) {
        count++;
      } else {
        count--;
      }

      startt1();

      if(turnstarttime != 0) {
        currentw = (float)whperturn/(((float)millis()-(float)turnstarttime)/3600000.00);
      }

      turnstarttime = (float)millis();
    }
  }
  if (s2 && !lasts2) { //Positive Edge of s2
    if(!time2started && time1started) { //Start measuring t2 if it isn#t being measured already
      startt2();
      stopt1();
    }
  }

  //Update lasts1 and lasts2
  lasts1 = s1;
  lasts2 = s2;

  //Send out the Current Watt usage via Serial
  Serial.print(currentw);
  Serial.println();
}

void startt1() { //Start measuring t1
  time1start = millis();
  time1started = true;
}

void stopt1() { //Stop measuring t1
  time1 = millis() - time1start;
  time1set = true;
}

void startt2() { //Start measuring t2
  time2start = millis();
  time2started = true;
}

void stopt2() { //Stop measuring t2
  time2 = millis() - time2start;
  time2set = true;
}
