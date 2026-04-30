# 🚨 Industrial Gas Asset Monitoring System (STM32 + IoT + Digital Twin)

## 📌 Overview

This project is a **full-stack industrial safety system** designed to monitor gas pipelines in real-time using an STM32 microcontroller. It integrates multiple sensors, intelligent fault detection, a live SCADA dashboard, and a 3D digital twin for visualization.

The system is built to detect and classify hazardous conditions such as gas leaks, abnormal pressure, flow irregularities, and mechanical vibrations with a priority-based alert mechanism.

---

## ⚙️ Key Features

### 🔹 Embedded System (STM32)

* Multi-sensor integration:

  * MQ2 Gas Sensor
  * HX710B Pressure Sensor
  * Flow Sensor
  * DHT11 Temperature Sensor
  * Vibration Sensor
* 4-Level Priority Alarm System:

  * NORMAL
  * WARNING
  * CRITICAL
  * EMERGENCY
* Hysteresis-based thresholding to avoid false triggers
* Non-blocking buzzer patterns for different alert levels
* Auto-calibration of gas sensor using interrupt (push button)

---

### 🔹 Backend (Flask + Python)

* Real-time data acquisition via serial communication
* REST API + WebSocket (Socket.IO) support
* AI-based anomaly prediction (trained using sensor logs)
* Automatic Excel logging using Pandas & OpenPyXL

---

### 🔹 Frontend (React + SCADA Dashboard)

* Live sensor data visualization
* Real-time charts and alert indicators
* SCADA-style industrial UI
* WebSocket-based instant updates

---

### 🔹 3D Digital Twin

* Built using React Three Fiber / Three.js
* Real-time visualization of pipeline conditions
* Dynamic animations based on sensor data

---

## 🧱 System Architecture

```text
STM32 → Serial → Flask Backend → WebSocket → React Dashboard + Digital Twin
```

---

## 📁 Project Structure

```text
industrial-gas-monitoring-stm32-iot/
│
├── firmware/        # STM32 Embedded C code
├── backend/         # Flask server + AI + logging
├── frontend/        # React dashboard + digital twin
├── docs/            # diagrams, pipeline.html
├── data-logs/       # sensor logs (Excel/CSV)
└── README.md
```

---

## 🚀 Getting Started

### 🔹 Backend Setup

```bash
cd backend
pip install -r requirements.txt
python server.py
```

---

### 🔹 Frontend Setup

```bash
cd frontend
npm install
npm start
```

---

### 🔹 Firmware

* Open STM32 project in STM32CubeIDE
* Flash code to STM32F446RE board

---

## 📊 Technologies Used

* **Embedded:** STM32, Embedded C
* **Backend:** Flask, Socket.IO, Pandas, OpenPyXL
* **Frontend:** React.js
* **3D Visualization:** Three.js, React Three Fiber
* **AI/ML:** Python (Joblib models)

---

## 🎯 Applications

* Industrial gas pipeline monitoring
* Smart factories (Industry 4.0)
* Oil & gas safety systems
* Remote asset monitoring

---

## 📸 Screenshots (Add Yours)

* Dashboard UI
* Digital Twin View
* Hardware Setup

---

## 👨‍💻 Author

**Poobalashree S**

---

## ⭐ Future Improvements

* Cloud integration (AWS / Firebase)
* Mobile app alerts
* Predictive maintenance using advanced ML models
* GSM/GPS-based remote alert system

---

## 📜 License

This project is for educational and research purposes.
