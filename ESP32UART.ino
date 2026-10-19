#include "BluetoothSerial.h"
#include <IRrecv.h>
#include <IRutils.h>

const int IR=27; //IR sensor
IRrecv irrecv(IR);
decode_results results;
BluetoothSerial SerialBT; //for phone
HardwareSerial UART(2); //for arduino

void setup() {
  Serial.begin(115200);
  UART.begin(9600, SERIAL_8N1, 16, 17); //RX, TX
  SerialBT.begin("ESP32_BT");
  irrecv.enableIRIn();
}

void loop() {

  //Bluetooth
  while(UART.available()) {
    SerialBT.write(UART.read());
  }

  while(SerialBT.available()) {
    UART.write(SerialBT.read());
  }

  //IR remote
  if(irrecv.decode(&results)) {
    Serial.print("Protocol: ");
    Serial.println(typeToString(results.decode_type));

    Serial.print("Code: 0x");
    Serial.println(results.value, HEX);

    Serial.println();

    switch(results.value) {

      case 0xFF6897: //0
        UART.write('0');
        break;

      case 0xFF30CF: //1 
        UART.write('1');
        break;

      case 0xFF18E7: //2
        UART.write('2');
        break;

      case 0xFF7A85: //3
        UART.write('3');
        break;

      case 0xFF10EF: //4
        UART.write('4');
        break;

      case 0xFF38C7: //5
        UART.write('5');
        break;

      case 0xFF5AA5: //6
        UART.write('6');
        break;

      case 0xFF42BD: //7
        UART.write('7');
        break;

      case 0xFF4AB5: //8
        UART.write('8');
        break;

      case 0xFF52AD: //9
        UART.write('9');
        break;
    }

    irrecv.resume();

  }
}
