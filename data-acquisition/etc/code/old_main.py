import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

import time

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

plt.show()

# count = 0
t = 0
while True:
    time_stmp.append(t)

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

    '''
        rawData = arduino.read()
        if rawData == b'$' :
            rawData = arduino.readline()

            data_pres[0].append( struct.unpack('f', rawData[0:4])[0] )
            data_pres[1].append( struct.unpack('f', rawData[4:8])[0] )
            data_pres[2].append( struct.unpack('f', rawData[8:12])[0] )
            data_pres[3].append( struct.unpack('f', rawData[12:16])[0] )
            data_pres[4].append( struct.unpack('f', rawData[16:20])[0] )
            data_pres[5].append( struct.unpack('f', rawData[20:24])[0] )
            
            data_temp[0].append( struct.unpack('f', rawData[24:28])[0] ) 
            data_temp[1].append( struct.unpack('f', rawData[28:32])[0] ) 
            data_temp[2].append( struct.unpack('f', rawData[32:36])[0] ) 
            data_temp[3].append( struct.unpack('f', rawData[36:40])[0] ) 
            data_temp[4].append( struct.unpack('f', rawData[40:44])[0] )

            time = np.arange(0, 60, 1)
    '''

    for i in range(6):
        pressure_subplot[i].plot(data_pres[i])
    for i in range(5):
        temperature_subplot[i].plot(data_temp[i])

    fig.canvas.draw()

    for i in range(6):
        pressure_subplot[i].set_xlim(left = max(0, i-15), right = i+5)
    for i in range(5):
        temperature_subplot[i].set_xlim(left = max(0, i-15), right = i+5)

    # time.sleep(0.1)
    # count += 1
    t += 1

plt.close()

    

    