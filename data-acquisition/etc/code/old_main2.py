import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

from itertools import count
import pandas as pd
from matplotlib.animation import FuncAnimation
plt.style.use('fivethirtyeight')

import serial
import struct


arduino = serial.Serial("COM10", 115200)
data_pres = [ [], [], [], [], [], [] ]
data_temp = [ [], [], [], [], [] ]
time_stmp = [ ]


fig = plt.figure(constrained_layout = True)
grid = gridspec.GridSpec(ncols = 6, nrows = 2, figure = fig)

pressure_subplot = [
    fig.add_subplot(grid[0,0]),
    fig.add_subplot(grid[0,1]),
    fig.add_subplot(grid[0,2]),
    fig.add_subplot(grid[0,3]),
    fig.add_subplot(grid[0,4]),
    fig.add_subplot(grid[0,5])
]
temperature_subplot = [
    fig.add_subplot(grid[1,0]),
    fig.add_subplot(grid[1,1]),
    fig.add_subplot(grid[1,2]),
    fig.add_subplot(grid[1,3]),
    fig.add_subplot(grid[1,4])
]

for i in range(6):
    pressure_subplot[i].set_title( "Pressure" + str(i+1) )
    pressure_subplot[i].set_xlabel( "Time (s) ")
    pressure_subplot[i].set_ylabel( "Pressure (kPa)" )
    # pressure_subplot[i].set_yticks([0, 1, 2, 3, 4, 5, 10])
    pressure_subplot[i].plot( data_pres[i], time_stmp )

for i in range(5):
    temperature_subplot[i].set_title("Temperature" + str(i+1) )
    temperature_subplot[i].set_xlabel( "Time (s) ")
    temperature_subplot[i].set_ylabel("Temperature (" + r"$\degree$" + "C)" )
    # temperature_subplot[i].set_yticks([-250, -100, 0, 100, 250])
    temperature_subplot[i].plot( data_temp[i], time_stmp )

def animate(i):
    for i in range(6):
        line = arduino.readline()
        if line:
            string = line.decode()
            val = float(string)
            print(val)
            data_pres[i].append(val)
    for i in range(5):
        line = arduino.readline()
        if line:
            string = line.decode()
            val = float(string)
            print(val)
            data_temp[i].append(val)


    for i in range(6):
        pressure_subplot[i].plot(data_pres[i])
    for i in range(5):
        temperature_subplot[i].plot(data_temp[i])


    x = data['x_value']
    y = data['total_1']

    ax = plt.gca()
    line = ax.lines

    line.set_data(x, y)

    xlim_low, xlim_high = ax.get_xlim()
    ylim_low, ylim_high = ax.get_ylim()

    for i in range(6):
        pressure_subplot[i].set_xlim( xlim_low, (x.max()+5) )
    for i in range(5):
        temperature_subplot[i].set_xlim( xlim_low, (x.max()+5) )


    ymax = y.max()
    current_ymax = ymax 

    ymin = y.min()
    current_ymin = ymin

    ax.set_ylim((current_ymin - 5), (current_ymax + 5))


ani = FuncAnimation(fig, animate, interval = 1000)

plt.legend()
plt.tight_layout()
plt.show()

