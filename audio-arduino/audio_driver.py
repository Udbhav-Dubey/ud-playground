import sounddevice as sd
devices = sd.query_devices()
for i, dev in enumerate(devices):
    print(f"{i}: {dev['name']} | Inputs: {dev['max_input_channels']} | Outputs: {dev['max_output_channels']}")
