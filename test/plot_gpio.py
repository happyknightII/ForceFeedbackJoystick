import serial
import matplotlib.pyplot as plt
from collections import deque

# === CONFIGURATION ===
SERIAL_PORT = "COM8"  # Update this to your actual port
BAUD_RATE = 115200
WINDOW_SIZE = 100  # Number of points to display
PLOT_REFRESH_RATE = 0.01  # Seconds between frame updates

# === SETUP ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
plt.ion()  # Interactive mode ON

# Deque to store voltage samples
voltage_data = deque([0.0] * WINDOW_SIZE, maxlen=WINDOW_SIZE)

# Initialize plot
fig, ax = plt.subplots()
(line,) = ax.plot(range(WINDOW_SIZE), voltage_data)
ax.set_ylim(0, 3.3)
ax.set_xlim(0, WINDOW_SIZE)
ax.set_title("Live Voltage Plot")
ax.set_xlabel("Sample")
ax.set_ylabel("Voltage (V)")
fig.tight_layout()

# === LOOP ===
try:
    while True:
        raw_line = ser.readline().decode(errors="ignore").strip()
        if raw_line.startswith("Voltage"):
            try:
                # Example line: "Voltage at GPIO 4: 2.45 V (raw: 3083)"
                parts = raw_line.split(":")[1].split("V")[0].strip()
                voltage = float(parts)
                voltage_data.append(voltage)

                # Update plot
                line.set_ydata(voltage_data)
                line.set_xdata(range(len(voltage_data)))
                ax.set_xlim(0, len(voltage_data))
                fig.canvas.draw()
                fig.canvas.flush_events()
                plt.pause(PLOT_REFRESH_RATE)
            except Exception as e:
                print(f"[Parse Error] '{raw_line}': {e}")

except KeyboardInterrupt:
    print("Exited.")
finally:
    ser.close()
