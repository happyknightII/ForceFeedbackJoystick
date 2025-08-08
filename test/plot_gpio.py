import serial
import matplotlib.pyplot as plt
from collections import deque

ser = serial.Serial("COM3", 115200)  # Replace COM3 with your port

window = 200
target = deque(maxlen=window)
actual = deque(maxlen=window)

plt.ion()
fig, ax = plt.subplots()
(line1,) = ax.plot([], [], label="Target (deg/s)")
(line2,) = ax.plot([], [], label="Actual (deg/s)")
ax.set_ylim(-200, 200)
ax.legend()

while True:
    line = ser.readline().decode().strip()
    if "," in line:
        try:
            t, a = map(float, line.split(","))
            target.append(t)
            actual.append(a)

            line1.set_ydata(target)
            line1.set_xdata(range(len(target)))
            line2.set_ydata(actual)
            line2.set_xdata(range(len(actual)))
            ax.relim()
            ax.autoscale_view()
            plt.pause(0.01)
        except:
            continue
