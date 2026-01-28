import subprocess
import numpy as np
import serial
import time
import sys
SERIAL_PORT = "/dev/ttyACM0"
BAUD = 115200
AUDIO_SOURCE = "alsa_output.pci-0000_08_00.6.HiFi__Speaker__sink.monitor"
ser = serial.Serial(SERIAL_PORT, BAUD)
time.sleep(2)
process = subprocess.Popen(
    ['parec', '--device', AUDIO_SOURCE, '--format=s16le', '--rate=48000', '--channels=2'],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    bufsize=4096
)
time.sleep(0.5)
if process.poll() is not None:
    stderr = process.stderr.read().decode()
    sys.exit(1)
chunk_size = 1024
counter = 0
smoothed_volume = 0
min_music = 300   
max_music = 5000
baseline_samples = []

try:
    while True:
        start_time=time.perf_counter()
        data = process.stdout.read(chunk_size)
        if not data:
            break
        
        audio = np.frombuffer(data, dtype=np.int16).astype(np.float32)
        rms = np.sqrt(np.mean(audio**2))
        
        if rms < min_music:
            volume = 0  
        else:
            volume = int(((rms - min_music) / (max_music - min_music)) * 255)
            volume = max(0, min(volume, 255))
        
        smoothed_volume = 0.6 * volume + 0.4 * smoothed_volume
        final_volume = int(smoothed_volume)
        
        ser.write(f"{final_volume}\n".encode())
        
        if counter % 10 == 0:
            bars = '█' * (final_volume // 16)  
            print(f"RMS: {rms:7.1f} | Brightness: {final_volume:3d} | {bars:<16}", end='\r')
        counter += 1
        end_time=time.perf_counter()
        latency=end_time-start_time
        print(f"Loop latency: {latency*1000:.3f} ms")
except KeyboardInterrupt:
    process.terminate()
    ser.write(b"0\n")
