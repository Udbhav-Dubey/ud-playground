import sounddevice as sd
import numpy as np
import serial
import time

SERIAL_PORT = "/dev/ttyACM0"
BAUD = 9600
DEVICE = 6   # pulse

ser = serial.Serial(SERIAL_PORT, BAUD)
time.sleep(2)

def callback(indata, frames, time_info, status):
    volume = int(np.linalg.norm(indata) * 12)
    volume = min(volume, 255)

    print("TX:", volume)
    ser.write(f"{volume}\n".encode())

with sd.InputStream(device=DEVICE, channels=2, callback=callback):
    print("🎶 Playing music, sending values...")
    input("Press Enter to stop\n")

