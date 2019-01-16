#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02};
IPAddress ip(192,168,44,166);

EthernetServer server(80); //Standart HTTP Port

#define s1a A8
#define s2a A9

#define empf1 150
#define empf2 100

#define smoothing 16
#define turnsperkwh 73

int whperturn = 1/turnsperkwh*1000;
int turnstarttime;
int currentw;

int standart1, standart2;

int s1afiltered, s2afiltered;

bool s1, s2, lasts1, lasts2;

int time1, time2, time1start, time2start;
bool time1started, time2started, time1set, time2set;

int count = 0;
int diff;

void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server ist Online, IP:");
  Serial.println(Ethernet.localIP());
  
  standart1 = analogRead(s1a);
  standart2 = analogRead(s2a);
  lasts1 = analogRead(s1a) - standart1 > empf1;
  lasts2 = analogRead(s2a) - standart2 > empf2;
}

void loop() {
  updatesensors();
  
  EthernetClient client = server.available();

  if (client) {
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close"); 
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("Aktuelle Leistung: ");
          client.print(currentw);
          client.print(" || Umdrehungen seit Start: ");
          client.print(count);
          client.println("</html>");
          break;
        }
        
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
  for (int i = 0; i < smoothing; ++i) {
    s1afiltered += analogRead(s1a);
    s2afiltered += analogRead(s2a);
    delay(1);
  }
  s1afiltered = s1afiltered / smoothing;
  s2afiltered = s2afiltered / smoothing;
  
  s1 = s1afiltered - standart1 > empf1;
  s2 = s2afiltered - standart2 > empf2;

  /**
  if (s1 && !lasts1) {
    if(s2 != s1) {
      count++;
    } else {
      count--;
    }
  }
  **/

  if (s1 && !lasts1) {
    if(!time1started) {
      startt1();
    } else if (time2started && time1set) {
      stopt2();
      
      time1set = false;
      time2set = false;
      time1started = false;
      time2started = false;
      
      if (time1 < time2) {
        count--;
      } else {
        count++;
      }
      
      startt1();
      
      if(turnstarttime != 0) {
        currentw = whperturn/((millis()-turnstarttime)/3600000);
      }
      
      turnstarttime = millis();
    }
  }
  if (s2 && !lasts2) {
    if(!time2started && time1started) {
      startt2();
      stopt1();
    }
  }
  

  lasts1 = s1;
  lasts2 = s2;

  Serial.print(s1afiltered);
  Serial.print(" || ");
  Serial.print(s2afiltered);
  Serial.print(" || ");
  Serial.print(s1);
  Serial.print(" || ");
  Serial.print(s2);
  Serial.print(" || ");
  Serial.print(s1afiltered - standart1);
  Serial.print(" || ");
  Serial.print(s2afiltered - standart2);
  Serial.print(" || ");
  Serial.print(count);
  Serial.print(" || ");
  Serial.println(currentw);
}

void startt1() {
  time1start = millis();
  time1started = true;
}

void stopt1() {
  time1 = millis() - time1start;
  time1set = true;
}

void startt2() {
  time2start = millis();
  time2started = true;
}

void stopt2() {
  time2 = millis() - time2start;
  time2set = true;
}
