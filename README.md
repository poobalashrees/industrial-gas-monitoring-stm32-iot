# Industrial Gas Monitoring System (STM32 + IoT)

## 📌 Overview

A real-time industrial safety system designed to detect gas leaks, pressure anomalies, abnormal flow, and vibration events using a multi-sensor embedded architecture.

## ⚙️ Features

* Multi-sensor integration (Gas, Pressure, Flow, Temperature, Vibration)
* Priority-based 4-level alarm system
* Hysteresis thresholding for stable detection
* Auto-calibration using hardware interrupt (PC13)
* Non-blocking PWM buzzer alert patterns
* Real-time monitoring capability

## 🛠️ Tech Stack

* STM32 F446RE (Embedded C)
* STM32 HAL Drivers
* Sensors: MQ2, HX710B, DHT11, Flow Sensor

## 🔌 Hardware Components

* STM32 Nucleo F446RE
* MQ2 Gas Sensor
* HX710B Pressure Sensor
* DHT11 Temperature Sensor
* Flow Sensor
* Servo Motor (Gas Valve Control)

## 📂 Project Structure

* `/Core/Src` → Application source files
* `/Core/Inc` → Header files
* `/Startup` → MCU startup code
* `.ioc` → STM32CubeMX configuration

## 📷 Demo

![Setup](images/your_image_name.jpg)

## 🚀 Future Improvements

* IoT dashboard integration
* Edge AI for anomaly detection
* Cloud monitoring system
