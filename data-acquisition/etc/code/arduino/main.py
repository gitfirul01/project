import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np


fig = plt.figure(constrained_layout = True)
grid = gridspec.GridSpec(ncols = 6, nrows = 2, figure = fig)

data = { 'pressure':
            {
                1 : np.random.randint(0, 10, 60), 
                2 : np.random.randint(0, 10, 60), 
                3 : np.random.randint(0, 10, 60), 
                4 : np.random.randint(0, 10, 60), 
                5 : np.random.randint(0, 10, 60), 
                6 : np.random.randint(0, 10, 60)
            },
         'temperature':
            {
                1 : np.random.randint(0, 250, 60), 
                2 : np.random.randint(0, 250, 60), 
                3 : np.random.randint(0, 250, 60), 
                4 : np.random.randint(0, 250, 60), 
                5 : np.random.randint(0, 250, 60)
            }
        }

time = np.arange(0, 60, 1)

##
p1 = fig.add_subplot(grid[0,0])
p2 = fig.add_subplot(grid[0,1])
p3 = fig.add_subplot(grid[0,2])
p4 = fig.add_subplot(grid[0,3])
p5 = fig.add_subplot(grid[0,4])
p6 = fig.add_subplot(grid[0,5])

t1 = fig.add_subplot(grid[1,0])
t2 = fig.add_subplot(grid[1,1])
t3 = fig.add_subplot(grid[1,2])
t4 = fig.add_subplot(grid[1,3])
t5 = fig.add_subplot(grid[1,4])

###
p1.set_title("Pressure 1")
p2.set_title("Pressure 2")
p3.set_title("Pressure 3")
p4.set_title("Pressure 4")
p5.set_title("Pressure 5")
p6.set_title("Pressure 6")

t1.set_title("Temperature 1")
t2.set_title("Temperature 2")
t3.set_title("Temperature 3")
t4.set_title("Temperature 4")
t5.set_title("Temperature 5")

###
p1.plot(time, data['pressure'][1])
p2.plot(time, data['pressure'][2])
p3.plot(time, data['pressure'][3])
p4.plot(time, data['pressure'][4])
p5.plot(time, data['pressure'][5])
p6.plot(time, data['pressure'][6])

t1.plot(time, data['temperature'][1])
t2.plot(time, data['temperature'][2])
t3.plot(time, data['temperature'][3])
t4.plot(time, data['temperature'][4])
t5.plot(time, data['temperature'][5])

###
p1.set_xlabel('Time (s)')
p2.set_xlabel('Time (s)')
p3.set_xlabel('Time (s)')
p4.set_xlabel('Time (s)')
p5.set_xlabel('Time (s)')
p6.set_xlabel('Time (s)')

t1.set_xlabel('Time (s)')
t2.set_xlabel('Time (s)')
t3.set_xlabel('Time (s)')
t4.set_xlabel('Time (s)')
t5.set_xlabel('Time (s)')

##
p1.set_ylabel('Pressure (kPa)')
p2.set_ylabel('Pressure (kPa)')
p3.set_ylabel('Pressure (kPa)')
p4.set_ylabel('Pressure (kPa)')
p5.set_ylabel('Pressure (kPa)')
p6.set_ylabel('Pressure (kPa)')

t1.set_ylabel('Temperature (' + r'$\degree$' + 'C)')
t2.set_ylabel('Temperature (' + r'$\degree$' + 'C)')
t3.set_ylabel('Temperature (' + r'$\degree$' + 'C)')
t4.set_ylabel('Temperature (' + r'$\degree$' + 'C)')
t5.set_ylabel('Temperature (' + r'$\degree$' + 'C)')


plt.show()