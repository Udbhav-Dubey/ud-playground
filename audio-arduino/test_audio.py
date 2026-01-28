import sounddevice as sd
import numpy as np

DEVICE = 6  # pulse

def callback(indata, frames, time, status):
    volume = int(np.linalg.norm(indata) * 10)
    print(volume)

with sd.InputStream(device=DEVICE, channels=2, callback=callback):
    input("Playing YouTube... press Enter to stop\n")

