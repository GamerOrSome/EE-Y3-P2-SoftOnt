import serial
import time
from pathlib import Path

# Open serial port
myport = serial.Serial(
    port='COM10',
    baudrate=115200,
    timeout=1
)

time.sleep(2) 

with open('Python/script.txt', "r", encoding="utf-8") as f:
    for line in f:
        # Strip any existing line endings and add proper CRLF
        command = line.strip()
        if command:  # Skip empty lines
            # Send command with CRLF terminator
            myport.write((command + "\r\n").encode("utf-8"))
            
            # Read response from STM32
            time.sleep(0.2)  # Increased delay to allow STM32 to process
            response = myport.readline().decode("utf-8", errors="ignore").strip()
            
            # Print the command and response
            print(f"Sent: {command}")
            print(f"Response: {response}")
            
            # Check if there was an error
            if response.startswith("ERR:"):
                print(f"ERROR detected: {response}")
            elif response == "OK":
                print("Command executed successfully")
            print("-" * 50)
            time.sleep(1)  # Short delay before sending the next command