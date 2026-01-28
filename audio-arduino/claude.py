import subprocess
import sys

print("🔍 Finding audio sources...\n")

# List all PulseAudio sources
result = subprocess.run(['pactl', 'list', 'sources', 'short'], 
                       capture_output=True, text=True)

print("Available sources:")
print(result.stdout)

print("\n" + "="*60)
print("Looking for monitor sources (these capture system audio):")
print("="*60)

sources = result.stdout.strip().split('\n')
monitor_sources = []

for i, line in enumerate(sources):
    if 'monitor' in line.lower():
        parts = line.split('\t')
        source_name = parts[1] if len(parts) > 1 else parts[0]
        monitor_sources.append(source_name)
        print(f"\n✅ Found: {source_name}")

if not monitor_sources:
    print("\n❌ No monitor sources found!")
    print("\nTry running: pactl load-module module-loopback")
else:
    print(f"\n\n💡 Use this source: {monitor_sources[0]}")
