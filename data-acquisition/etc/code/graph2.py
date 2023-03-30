import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import matplotlib.animation as animation
from matplotlib import interactive, style
from pandas.io.parsers import read_csv 

file_name = 'export_dataframe.csv'
read_path = r'D:\MyFiles\redesma\data-acquisition\main\{}'.format(file_name)

style.use('fivethirtyeight')
fig = plt.figure()
p1 = fig.add_subplot(6,2,1)
p2 = fig.add_subplot(6,2,2)
p3 = fig.add_subplot(6,2,3)
p4 = fig.add_subplot(6,2,4)
p5 = fig.add_subplot(6,2,5)
p6 = fig.add_subplot(6,2,6)

t1 = fig.add_subplot(6,2,7)
t2 = fig.add_subplot(6,2,8)
t3 = fig.add_subplot(6,2,9)
t4 = fig.add_subplot(6,2,10)
t5 = fig.add_subplot(6,2,11)


def animate():
    df = pd.read_csv(read_path)
    ys = df.iloc[1:, 2].values
    xs = list(range(1, len(ys)+1))

    ys = df.iloc[1:, 2].values
    p1.clear()
    p1.plot(xs, ys)
    
    ys = df.iloc[1:, 3].values
    p2.clear()
    p2.plot(xs, ys)

    ys = df.iloc[1:, 4].values
    p3.clear()
    p3.plot(xs, ys)

    ys = df.iloc[1:, 5].values
    p4.clear()
    p4.plot(xs, ys)

    ys = df.iloc[1:, 6].values
    p5.clear()
    p5.plot(xs, ys)

    ys = df.iloc[1:, 7].values
    p6.clear()
    p6.plot(xs, ys)

    ys = df.iloc[1:, 8].values
    t1.clear()
    t1.plot(xs, ys)

    ys = df.iloc[1:, 9].values
    t2.clear()
    t2.plot(xs, ys)

    ys = df.iloc[1:, 10].values
    t3.clear()
    t3.plot(xs, ys)

    ys = df.iloc[1:, 11].values
    t4.clear()
    t4.plot(xs, ys)

    ys = df.iloc[1:, 12].values
    t5.clear()
    t5.plot(xs, ys)


ani = animation.FuncAnimation(fig, animation, interval = 1000)

plt.tight_layout()
plt.show()