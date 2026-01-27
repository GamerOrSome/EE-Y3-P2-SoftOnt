import serial
import time
from pathlib import Path

# Open serial port
myport = serial.Serial(
    port='COM10',
    baudrate=115200,
    timeout=10
)

time.sleep(2) 

with open('Python/script.txt', "r", encoding="utf-8") as f:
    for line in f:
        # Strip any existing line endings and add proper CRLF
        command = line.strip()
        if command:  # Skip empty lines
            # Send command with CRLF terminator
            myport.write((command + "\r\n").encode("utf-8"))
            
            # Wait for echo response (command + \r\n)
            echo = myport.readline().decode("utf-8", errors="ignore").strip()
            
            # Wait for actual response (OK or ERR:...)
            response = myport.readline().decode("utf-8", errors="ignore").strip()
            
            # Print the command and response
            print(f"Sent: {command}")
            print(f"Echo: {echo}")
            print(f"Response: {response}")
            
            # Check if there was an error
            if response.startswith("ERR:"):
                print(f"ERROR detected: {response}")
            elif response == "OK":
                print("Command executed successfully")
            else:
                print(f"Unexpected response: {response}")
            print("-" * 50)