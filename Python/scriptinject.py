import serial
import time
from pathlib import Path

# Open serial port
myport = serial.Serial(
    port='COM11',
    baudrate=115200,
    timeout=1
)

time.sleep(2) 

with open('Python/script.txt', "r", encoding="utf-8") as f:
    for line in f:
        # Convert string → bytes
        myport.write(line.encode("utf-8"))