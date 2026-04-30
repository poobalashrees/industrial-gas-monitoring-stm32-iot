import serial
import csv
from datetime import datetime

ser = serial.Serial('COM8', 115200, timeout=1)

# Open CSV file
with open('data_log.csv', 'a', newline='') as file:
    writer = csv.writer(file)

    print("Logging started... Press CTRL+C to stop")

    while True:
        line = ser.readline().decode().strip()

        if line:
            print(line)

            data = line.split(',')

            if len(data) == 6:
                row = [datetime.now()] + data
                writer.writerow(row)