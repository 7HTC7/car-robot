#include "SoftwareSerial.h"

SoftwareSerial mySerial(10, 11); //RX, TX

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600);
}

void loop() {
  while(mySerial.available()) {
    Serial.write(mySerial.read());
  }

  while(Serial.available()) {
    mySerial.write(Serial.read());
  }
  
}