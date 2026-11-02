# Multi-Mode Robot Car

A multi-mode robot car built with Arduino and ESP32, featuring line following, ultrasonic obstacle avoidance, Bluetooth control, and IR remote control. 
The robot uses multiple sensors, a servo motor, RGB LED indicators, a buzzer, and an INA226 sensor for battery monitoring.

## Features

- Line following using four IR sensors
- Obstacle avoidance using an ultrasonic sensor and servo motor
- Wireless control via Bluetooth (through ESP32)
- IR remote control (through ESP32)
- Multiple operating modes selected using a physical button
- RGB LED status indication
- Buzzer feedback and low-battery warning
- INA226 voltage and current monitoring

## Hardware

- Arduino-compatible board and sensor shield
- ESP32
- DC motors and motor driver
- IR line sensors
- Ultrasonic sensor
- Servo motor
- Bluetooth module
- IR receiver and remote
- INA226 sensor
- RGB LED, buzzer, and mode button
- Batteries

## Operating Modes

1. Idle
2. IR Remote Control
3. Line Following
4. Obstacle Avoidance
5. Maze Solver *(planned)*
6. Bluetooth Control

## Communication

The Arduino and ESP32 communicate through UART.

- Arduino RX: D0
- Arduino TX: D1
- ESP32 RX: GPIO 16
- ESP32 TX: GPIO 17
- ESP32 IR Receiver: GPIO 27

