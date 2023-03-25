import os
import serial
import csv
from datetime import datetime

# Create the directory to save the CSV files in
logs_dir = 'SerialMonitorLogs'
if not os.path.exists(logs_dir):
    os.makedirs(logs_dir)

# Set up the serial connection
# replace COM3 with the serial port of Arduino
arduino_mega = serial.Serial('COM3', 9600)

# Set up the CSV writer and header row
# Get the current date
current_date = datetime.now().strftime('%Y-%m-%d')  
header = [current_date, 'CO2', 'Air Quality', 'Oxygen', 'Water Level']  
file_name = f'{logs_dir}/{current_date}.csv'  
with open(file_name, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(header)

# Read data from the serial port and write to the CSV file
while True:
    if arduino_mega.in_waiting > 0:
        data = arduino_mega.readline().decode().rstrip()
        with open(file_name, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([data])
