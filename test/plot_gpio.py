import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# ===== CONFIGURATION =====
SERIAL_PORT = "COM8"  # Change to your ESP32 port
BAUD_RATE = 115200
MAX_POINTS = 5000  # How many points to keep on screen

# ===== INITIALIZATION =====
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time_data = deque(maxlen=MAX_POINTS)
level_data = deque(maxlen=MAX_POINTS)

fig, ax = plt.subplots()
(line,) = ax.plot([], [], lw=2)
ax.set_ylim(-0.2, 1.2)
ax.set_xlabel("Time (ms)")
ax.set_ylabel("GPIO Level")
ax.set_title("Live GPIO Level Plot")


def update(frame):
    while ser.in_waiting:
        try:
            line_raw = ser.readline().decode().strip()
            if line_raw.startswith("timestamp_us"):
                continue  # header
            if "," not in line_raw:
                continue
            t_us, level = map(int, line_raw.split(","))
            time_data.append(t_us / 1000.0)  # convert to ms
            level_data.append(level)
        except Exception as e:
            print("Parse error:", e)

    if time_data:
        line.set_data(time_data, level_data)
        ax.set_xlim(time_data[0], time_data[-1])
    return (line,)


ani = animation.FuncAnimation(fig, update, interval=50)
plt.tight_layout()
plt.show()
