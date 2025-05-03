import serial
import re
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# Adjust this to match your serial port and baud rate
SERIAL_PORT = "COM8"  # or '/dev/ttyUSB0' on Linux/Mac
BAUD_RATE = 115200

# Buffer size for plotting
MAX_POINTS = 500

# Compile regex to extract float value
angle_regex = re.compile(r"Angle:\s+([-+]?[0-9]*\.?[0-9]+)")

# Initialize serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Data buffers
angle_data = deque(maxlen=MAX_POINTS)
time_data = deque(maxlen=MAX_POINTS)
time_counter = 0


def update_plot(frame):
    global time_counter
    while ser.in_waiting:
        line = ser.readline().decode(errors="ignore").strip()
        match = angle_regex.search(line)
        if match:
            angle = float(match.group(1))
            angle_data.append(angle)
            time_data.append(time_counter)
            time_counter += 1

    ax.clear()
    ax.plot(time_data, angle_data, label="Angle (degrees)")
    ax.set_ylim(0, 360)
    ax.set_xlabel("Time (samples)")
    ax.set_ylabel("Angle (°)")
    ax.set_title("Real-Time Encoder Angle")
    ax.legend()
    ax.grid(True)


fig, ax = plt.subplots()
ani = animation.FuncAnimation(fig, update_plot, interval=10)
plt.tight_layout()
plt.show()
