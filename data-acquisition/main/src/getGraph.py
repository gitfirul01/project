#!../venv/Scripts/python
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import matplotlib.animation as animation
from matplotlib import style
from matplotlib import ticker
from matplotlib.backend_bases import MouseButton
import yaml

with open("./config.yml", "r") as config:
    cfg = yaml.load(config, Loader=yaml.SafeLoader)

x_thresh = cfg["graph"]["x_threshold"]
custom_x_dataframe = cfg["graph"]["custom_x_dataframe"]
custom_yp_dataframe = cfg["graph"]["custom_yp_dataframe"]
custom_yt_dataframe = cfg["graph"]["custom_yt_dataframe"]
pressure_color = cfg["graph"]["pressure_color"]
temperature_color = cfg["graph"]["temperature_color"]
yp_min = cfg["graph"]["yp_min"]
yp_max = cfg["graph"]["yp_max"]
yt_min = cfg["graph"]["yt_min"]
yt_max = cfg["graph"]["yt_max"]
interval = cfg["graph"]["interval"]

file_name = cfg["file"]["name"]
read_path = './{}.csv'.format(file_name)

# make plot template
style.use('fivethirtyeight')
fig = plt.figure('Data Acquisition - by redesma.id', constrained_layout = True)
grid = gridspec.GridSpec(ncols = 6, nrows = 2, figure = fig)
yformatter = ticker.FormatStrFormatter('%1.2f')
xformatter = ticker.FormatStrFormatter('%d')
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

def addSecondFigure(index, name, color):
    d = pd.read_csv(read_path)
    y = d.iloc[1:, index].values
    x = list(range(1, len(y)+1))
    
    fig2 = plt.figure(name)
    ax2 = fig2.add_subplot(111)
    ax2.plot(x, y, color=color, linewidth=1)
    ax2.tick_params(axis='both', labelsize=10)
    fig2.show()

def on_click(event):
    if event.button is MouseButton.LEFT:
        if event.inaxes == pressure_subplot[0]:
            addSecondFigure(1, 'Pressure 1', pressure_color)
        if event.inaxes == pressure_subplot[1]:
            addSecondFigure(2, 'Pressure 2', pressure_color)
        if event.inaxes == pressure_subplot[2]:
            addSecondFigure(3, 'Pressure 3', pressure_color)
        if event.inaxes == pressure_subplot[3]:
            addSecondFigure(4, 'Pressure 4', pressure_color)
        if event.inaxes == pressure_subplot[4]:
            addSecondFigure(5, 'Pressure 5', pressure_color)
        if event.inaxes == pressure_subplot[5]:
            addSecondFigure(6, 'Pressure 6', pressure_color)
        if event.inaxes == temperature_subplot[0]:
            addSecondFigure(7, 'Temperature 1', temperature_color)
        if event.inaxes == temperature_subplot[1]:
            addSecondFigure(8, 'Temperature 2', temperature_color)
        if event.inaxes == temperature_subplot[2]:
            addSecondFigure(9, 'Temperature 3', temperature_color)
        if event.inaxes == temperature_subplot[3]:
            addSecondFigure(10, 'Temperature 4', temperature_color)
        if event.inaxes == temperature_subplot[4]:
            addSecondFigure(11, 'Temperature 5', temperature_color)

def animate(i):
    # read value from file
    df = pd.read_csv(read_path)
    ys = df.iloc[1:, 1].values
    xs = list(range(1, len(ys)+1))

    # set x-axis ticks
    if (len(xs)-x_thresh < 0):
        x_min = 0 
    else:
        x_min = len(xs)-x_thresh
    x_max = len(xs)
    
    # iterate for plotting
    for i in range(6):
        ys = df.iloc[1:, i+1].values
        
        pressure_subplot[i].clear()
        pressure_subplot[i].plot(xs, ys, color=pressure_color, linewidth=1)
        pressure_subplot[i].yaxis.set_major_formatter(yformatter)
        pressure_subplot[i].xaxis.set_major_formatter(xformatter)
        if custom_x_dataframe:
            pressure_subplot[i].set_xlim(x_min, x_max)
        if custom_yp_dataframe:
            pressure_subplot[i].set_ylim(yp_min, yp_max)
        pressure_subplot[i].tick_params(axis='both', labelsize=8)
        pressure_subplot[i].set_title("Pressure " + str(i+1), fontsize=11)
        pressure_subplot[i].set_xlabel("Time (s) ", fontsize=10)
        pressure_subplot[0].set_ylabel("Pressure (kPa)" , fontsize=10)

    for i in range(5):
        ys = df.iloc[1:, i+7].values

        temperature_subplot[i].clear()
        temperature_subplot[i].plot(xs, ys, color=temperature_color, linewidth=1)
        temperature_subplot[i].yaxis.set_major_formatter(yformatter)
        temperature_subplot[i].xaxis.set_major_formatter(xformatter)

        if custom_x_dataframe:
            temperature_subplot[i].set_xlim(x_min, x_max)
        if custom_yt_dataframe:
            temperature_subplot[i].set_ylim(yt_min, yt_max)
        temperature_subplot[i].tick_params(axis='both', labelsize=8)
        temperature_subplot[i].set_title("Temperature " + str(i+1), fontsize=11)
        temperature_subplot[i].set_xlabel("Time (s) ", fontsize=10)
        temperature_subplot[0].set_ylabel("Temperature (" + r"$\degree$" + "C)", fontsize=10)
        
# main animation function
ani = animation.FuncAnimation(fig, animate, interval = interval)
fig.canvas.mpl_connect('button_press_event', on_click)
# plt.connect('button_press_event', on_click)
plt.get_current_fig_manager().window.state('zoomed')
plt.show()