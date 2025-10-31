# 🚨 Smart Accident Detection and Alert System

## Overview
An IoT-based system that detects vehicle accidents in real time using Arduino Nano 33 IoT, and automatically sends location data to a Raspberry Pi dashboard via MQTT.

## Hardware
- Arduino Nano 33 IoT (built-in Wi-Fi + IMU)
- NEO-6M GPS module
- Raspberry Pi 4 (Flask + Mosquitto)
- Piezo buzzer
- Jumper wires, Li-ion batteries

## Software
- Arduino IDE
- Python 3.11 on Raspberry Pi
- Flask, paho-mqtt, geopy, gpiozero
- Mosquitto MQTT broker

## Run Instructions
### Raspberry Pi
```bash
sudo systemctl start mosquitto
python3 main.py
