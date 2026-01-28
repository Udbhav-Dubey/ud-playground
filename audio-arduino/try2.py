import sounddevice as sd
import numpy as np
import serial
import time

SERIAL_PORT = "/dev/ttyACM0"
BAUD = 115200
DEVICE = 6  # pulse

ser = serial.Serial(SERIAL_PORT, BAUD)
time.sleep(2)

# Adaptive noise floor
noise_floor = 280
recent_silence = []

def callback(indata, frames, time_info, status):
    global noise_floor
    
    # Calculate RMS
    rms = np.sqrt(np.mean(indata**2)) * 1000
    
    # Remove noise floor
    volume = max(0, rms - noise_floor)
    
    # Adaptive threshold: if volume is very low, track it as potential noise
    if volume < 20:
        recent_silence.append(rms)
        if len(recent_silence) > 50:  # Keep last 50 silent samples
            recent_silence.pop(0)
            # Update noise floor to average of recent silence
            noise_floor = np.mean(recent_silence)
    
    # Scale and limit
    volume = int(volume * 1.5)  # Amplify signal
    volume = min(volume, 255)
    
    # Send to Arduino
    ser.write(f"{volume}\n".encode())
    
    # DEBUG: Print every 10th callback
    if callback.counter % 10 == 0:
        print(f"Raw RMS: {rms:6.2f} | Noise floor: {noise_floor:6.2f} | Volume sent: {volume:3d}")
    
    callback.counter += 1

callback.counter = 0

with sd.InputStream(device=DEVICE, channels=2, callback=callback, 
                     blocksize=1024, samplerate=44100):
    print("🎶 Music → LEDs (adaptive noise cancellation)")
    print(f"Using device: {DEVICE}")
    print("Initial noise floor: 280")
    input("Press Enter to stop\n")
