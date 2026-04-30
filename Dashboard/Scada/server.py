from flask import Flask, jsonify, send_from_directory
from flask_socketio import SocketIO
from flask_cors import CORS
import threading
import time
import serial
import random
import joblib
import pandas as pd
import os

# -------- APP --------
app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# -------- LOAD AI MODEL --------
try:
    model = joblib.load("model.pkl")
    print("🤖 AI Model Loaded")
except:
    model = None
    print("⚠ AI Model not found")

# -------- SERIAL --------
try:
    ser = serial.Serial('COM8', 115200, timeout=1)
    print("✅ Serial Connected")
except:
    ser = None
    print("⚠ Simulation mode")

# -------- DATA --------
latest_data = {
    "gas": 0,
    "temp": -1,
    "pressure": 0,
    "flow": 0,
    "vibration": 0,
    "alarm": 0,
    "status": "SAFE",
    "ai_status": "SAFE",
    "event": "NORMAL",
    "timestamp": time.time()
}

# -------- RULE ENGINE --------
def evaluate_alarm(d):
    gas = d["gas"]
    pressure = d["pressure"]
    flow = d["flow"]
    temp = d["temp"]

    # 🔥 TEMP VALIDATION
    valid_temp = temp != -1

    # -------- GAS --------
    if gas > 400:
        return "CRITICAL"
    elif gas > 250:
        return "WARNING"

    # -------- FLOW --------
    if flow == 0:
        return "LOW_FLOW"

    # -------- PRESSURE --------
    if pressure > 20:
        return "WARNING"

    # -------- TEMP (ONLY IF CONNECTED) --------
    if valid_temp:
        if temp > 80 or temp < 0:
            return "WARNING"

    return "SAFE"

# -------- AI PREDICTION --------
def predict_ai(data):
    if model is None:
        return "SAFE"

    try:
        # 🔥 HANDLE TEMP PROPERLY
        temp = data["temp"]

        # Replace invalid temp with neutral value (or training mean)
        if temp == -1:
            temp = 25  # safe default

        X = [[
            data["gas"],
            data["pressure"],
            data["flow"],
            temp
        ]]

        pred = model.predict(X)[0]

        if pred == 2:
            return "CRITICAL"
        elif pred == 1:
            return "WARNING"
        else:
            return "SAFE"

    except Exception as e:
        print("AI Error:", e)
        return "SAFE"

# -------- EXCEL LOGGING --------
def log_to_excel(data):
    file = "full_log.xlsx"

    # Show clean temp in logs
    temp_display = data["temp"] if data["temp"] != -1 else "N/A"

    log_entry = {
        "Date": time.strftime("%Y-%m-%d"),
        "Time": time.strftime("%H:%M:%S"),
        "Timestamp": data["timestamp"],

        "Gas": data["gas"],
        "Pressure": data["pressure"],
        "Flow": data["flow"],
        "Temp": temp_display,
        "Vibration": data["vibration"],

        "System_Status": data["status"],
        "AI_Status": data["ai_status"],
        "Event": data["event"]
    }

    df = pd.DataFrame([log_entry])

    try:
        if os.path.exists(file):
            with pd.ExcelWriter(file, mode='a', engine='openpyxl', if_sheet_exists='overlay') as writer:
                df.to_excel(writer, index=False, header=False, startrow=writer.sheets['Sheet1'].max_row)
        else:
            df.to_excel(file, index=False)
    except Exception as e:
        print("❌ Excel error:", e)

# -------- DATA THREAD --------
def read_serial():
    global latest_data

    print("🚀 Data thread running")

    while True:
        try:
            # -------- READ DATA --------
            if ser:
                line = ser.readline().decode(errors='ignore').strip()
                if not line:
                    continue
            else:
                # SIMULATION (TEMP = -1 meaning disconnected)
                line = f"{random.randint(80,450)},-1,{random.randint(5,25)},{random.uniform(0,3):.2f},0,0"
                time.sleep(1)

            values = line.split(',')

            if len(values) != 6:
                continue

            data = {
                "gas": float(values[0]),
                "temp": float(values[1]),
                "pressure": float(values[2]),
                "flow": float(values[3]),
                "vibration": int(values[4]),
                "alarm": int(values[5]),
                "timestamp": time.time()
            }

            # -------- STATUS --------
            rule_status = evaluate_alarm(data)
            ai_status = predict_ai(data)

            # -------- FINAL DECISION --------
            if "CRITICAL" in [rule_status, ai_status]:
                final_status = "CRITICAL"
            elif "WARNING" in [rule_status, ai_status]:
                final_status = "WARNING"
            elif rule_status == "LOW_FLOW":
                final_status = "LOW_FLOW"
            else:
                final_status = "SAFE"

            # -------- EVENT --------
            event = "NORMAL"
            if final_status in ["WARNING", "CRITICAL", "LOW_FLOW"]:
                event = "ALERT"

            data["status"] = final_status
            data["ai_status"] = ai_status
            data["event"] = event

            latest_data = data

            print("📡", data)

            # -------- LOG --------
            log_to_excel(data)

            # -------- EMIT --------
            socketio.emit("update", data)

        except Exception as e:
            print("❌ Error:", e)

# -------- ROUTES --------
@app.route('/')
def index():
    return send_from_directory('.', 'pipeline.html')

@app.route('/data')
def get_data():
    return jsonify(latest_data)

# -------- START THREAD --------
threading.Thread(target=read_serial, daemon=True).start()

# -------- RUN --------
if __name__ == '__main__':
    print("🔥 Server running at http://127.0.0.1:5000")
    socketio.run(app, host='0.0.0.0', port=5000)