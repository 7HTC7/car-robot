#include <Servo.h>
#include <Arduino.h>
#include <Wire.h>
#include "INA226.h"

//MODE_BUTTON
const int modeButton=A1;
bool rawState=HIGH;
bool lastRawState=HIGH;
bool stableState=HIGH;
const unsigned long confirmationTime=2000;
unsigned long lastButtonPress=0;
bool modeConfirmed=false;

//RGB_LED
const int RED=7;
const int GREEN=8;
const int BLUE=9;

///INA226
INA226 ina(0x44);

//IR Sensors
const int IR1=10;
const int IR2=2;
const int IR3=12;
const int IR4=13;

//DC Motors
const int IN1=3;
const int IN2=5;
const int IN3=6;
const int IN4=11;

//Servo Motor
const int SERVO=A0;
const int LOOK_LEFT=30;
const int LOOK_CENTER=90;
const int LOOK_RIGHT=150;
const int SAFE_DISTANCE=30;
Servo servo;

//Buzzer
const int buzzer=4;

//Ultrasonic
const int Trig=A2;
const int Echo=A3;

enum Mode {
  IDLE,
  REMOTE,
  LINE_FOLLOW,
  OBSTACLE,
  MAZE,
  BLUETOOTH
};

Mode currentMode=IDLE;
int modeCounter=0;

const int SPEED_FAST=220;
const int SPEED_NORMAL=180;
const int SPEED_SLOW=100;
#define LOW_BATTERY_VOLTAGE 3

//--------------------------------------------------------
void setup() {
  Serial.begin(9600);

  //Ultrasonic
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  //Servo Motor
  servo.attach(SERVO);
  servo.write(LOOK_CENTER);

  //DC Motors
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  //IR Sensors
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);

  //RGB_LED
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(RED, HIGH);  //led is white initially
  digitalWrite(BLUE, HIGH);
  digitalWrite(GREEN, HIGH);

  //Mode Button
  pinMode(modeButton, INPUT_PULLUP);

  //INA226
  Wire.begin();
  ina.begin();
}

//--------------------------------------------------------

//Servo Motor
void lookAt(int angle) {
  servo.write(angle);
  delay(150);
}

//--------------------------------------------------------
//DC Motors

void rotateLeft(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(IN2, speed);
  analogWrite(IN3, speed);
}

void rotateRight(int speed) {
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  analogWrite(IN1, speed);
  analogWrite(IN4, speed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward(int leftSpeed, int rightSpeed) {
  digitalWrite(IN2, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(IN1, leftSpeed);
  analogWrite(IN3, rightSpeed);
}

void backward(int leftSpeed, int rightSpeed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  analogWrite(IN2, leftSpeed);
  analogWrite(IN4, rightSpeed);
}

void turnSlightLeft() {
  forward(SPEED_NORMAL, SPEED_FAST);
}

void turnSharpLeft() {
  forward(SPEED_SLOW, SPEED_FAST);
}

void turnSlightRight() {
  forward(SPEED_FAST, SPEED_NORMAL);
}

void turnSharpRight() {
  forward(SPEED_FAST, SPEED_SLOW);
}

//--------------------------------------------------------
//Ultrasonic

int computeDistance() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);

  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  unsigned long timeout=micros();

  while(digitalRead(Echo)==LOW) {
    if(micros()-timeout>30000) return -1;  //no echo detected for 30ms, give up
  }

  unsigned long start=micros();

  while(digitalRead(Echo)==HIGH) {
    if(micros()-start>30000) return -1;  //echo too long
  }

  unsigned long stop=micros();

  unsigned long echoTime=stop-start;
  unsigned long distance=echoTime/58;

  return distance;
}

void obstacleAvoidance() {
  int center=computeDistance();

  if (center==-1) { //-1 <=> measurement failure
    stopMotors();
    return;
  }

  if(center>SAFE_DISTANCE) { 
    forward(SPEED_NORMAL, SPEED_NORMAL);
    return;
  }

  stopMotors();  //obstacle found

  lookAt(LOOK_LEFT);
  int left=computeDistance();

  lookAt(LOOK_RIGHT);
  int right=computeDistance();

  lookAt(LOOK_CENTER);

  if(left==-1) left=0;
  if(right==-1) right=0;

  if(left<=SAFE_DISTANCE && right<=SAFE_DISTANCE) { //both directions are blocked
    backward(SPEED_NORMAL, SPEED_NORMAL);
    delay(250);
    stopMotors();
    return;
  }

  if(left>right) {
    rotateLeft(SPEED_NORMAL);
    delay(350);
  }
  else {
    rotateRight(SPEED_NORMAL);
    delay(350);
  }


  stopMotors();
}

//--------------------------------------------------------
//INA226
float getBatteryVoltage() {
  return ina.getBusVoltage();
}

float getMotorCurrent() {
  float shunt_mV=ina.getShuntVoltage_mV();

  //R100=0.1 ohms
  return (shunt_mV/1000.0)/0.1;
}

float getMotorPower() {
  return getBatteryVoltage()*getMotorCurrent();
}

void checkBattery() {
  float voltage=getBatteryVoltage();
  
  if(voltage<LOW_BATTERY_VOLTAGE) {
    setRedLED();
    tone(buzzer, 350);
  }

  else {
    noTone(buzzer);
  }
}

//--------------------------------------------------------
//IR Sensors

void lineFollower() {
  int s1=digitalRead(IR1);
  int s2=digitalRead(IR2);
  int s3=digitalRead(IR3);
  int s4=digitalRead(IR4);

  byte pattern=(s4<<3) | (s3<<2) | (s2<<1) | s1;

  switch (pattern) {
    case 0b0110:
      forward(SPEED_NORMAL, SPEED_NORMAL);
      break;

    case 0b0010:
      turnSlightRight();
      break;

    case 0b0001:
      turnSharpRight();
      break;

    case 0b0100:
      turnSlightLeft();
      break;

    case 0b1000:
      turnSharpLeft();
      break;

    case 0b1111:
      stopMotors();  //intersection
      break;

    case 0b0000:
      stopMotors();  //lost line
      break;

    default:
      forward(SPEED_NORMAL, SPEED_NORMAL);
  }
}

//--------------------------------------------------------
//Mode Button

void readModeButton() {
  static unsigned long debounceTime=0;
  rawState=digitalRead(modeButton);

  if (rawState!=lastRawState) debounceTime=millis();

  if (millis()-debounceTime>30) {
    if (rawState!=stableState) {
      stableState=rawState;

      if(stableState==LOW) {
        stopMotors(); //for safety
        modeCounter++;
        if (modeCounter>5) modeCounter=0;

        setStaticRGB((Mode)modeCounter);
        lastButtonPress=millis();
        modeConfirmed=false;
      }
    }
  }

  lastRawState=rawState;

  if (!modeConfirmed && (millis()-lastButtonPress>=confirmationTime)) {
    currentMode=(Mode) modeCounter;
    modeConfirmed=true;

    stopMotors(); //for safety
    setStaticRGB(currentMode);
    beepModeConfirmed();
  }
}

//--------------------------------------------------------
//Buzzer

void beepModeConfirmed() {
  for (int i=0; i<2; i++) {
    tone(buzzer, 1000);
    delay(150);
    noTone(buzzer);
    delay(100);
  }
}

void beepMazeSolved() {
  for (int i=0; i<2; i++) {
    tone(buzzer, 523);
    delay(120);

    tone(buzzer, 659);
    delay(120);

    tone(buzzer, 784);
    delay(180);

    noTone(buzzer);
    delay(500);
  }
}

//--------------------------------------------------------

void mazeSolver() {

}

//--------------------------------------------------------

void bluetoothControl() {
  if(Serial.available()>0) {
    char command=Serial.read();
    if(command=='\r' || command=='\n') {
      return;
    }

    switch(command) {
      case '0':
        forward(SPEED_NORMAL, SPEED_NORMAL);
        break;

      case '1':
        backward(SPEED_NORMAL, SPEED_NORMAL);
        break;

      case '2':
        rotateLeft(SPEED_NORMAL);
        break;

      case '3':
        rotateRight(SPEED_NORMAL);
        break;

      case '4':
        stopMotors();
        break;

      default: break;
    }
  }
}

//--------------------------------------------------------

void remoteControl() {
  if(Serial.available()>0) {
    char command=Serial.read();

    switch(command) {
      case '0':
        forward(SPEED_NORMAL, SPEED_NORMAL);
        break;

      case '1':
        backward(SPEED_NORMAL, SPEED_NORMAL);
        break;

      case '2':
        rotateLeft(SPEED_NORMAL);
        break;

      case '3':
        rotateRight(SPEED_NORMAL);
        break;

      case '4':
        stopMotors();
        break;

      default: break;
    }
  }
}

//--------------------------------------------------------
//RGB LED

void setRedLED() {
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
}

void setStaticRGB(Mode mode) {
  switch (mode) {
    case IDLE:
    {  //WHITE
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      digitalWrite(BLUE, HIGH);
      break;
    }

    case REMOTE:
    {  //GREEN
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, HIGH);
      digitalWrite(BLUE, LOW);
      break;
    }

    case LINE_FOLLOW:
    {  //CYAN
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, HIGH);
      digitalWrite(BLUE, HIGH);
      break;
    }

    case OBSTACLE:
    {  //YELLOW
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      digitalWrite(BLUE, LOW);
      break;
    }

    case MAZE:
    {  //MAGENTA
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, LOW);
      digitalWrite(BLUE, HIGH);
      break;
    }

    case BLUETOOTH:
    {  //BLUE
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      digitalWrite(BLUE, HIGH);
      break;
    }
  }
}

//--------------------------------------------------------

void loop() {
  checkBattery();
  readModeButton();

  if(!modeConfirmed) { 
    stopMotors();
    return;
  }

  switch(currentMode) {
    case IDLE:            //0
      break;

    case REMOTE:          //1
      remoteControl();
      break;

    case LINE_FOLLOW:     //2
      lineFollower();
      break;

    case OBSTACLE:        //3
      obstacleAvoidance();
      break;

    case MAZE:            //4
      mazeSolver();
      break;
    
    case BLUETOOTH:       //5
      bluetoothControl();
      break;
  }
}
