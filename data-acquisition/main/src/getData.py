#!../venv/Scripts/python
import pandas as pd
import datetime
import serial
import yaml
from time import sleep
from random import uniform

with open("./config.yml", "r") as config:
    cfg = yaml.load(config, Loader=yaml.SafeLoader)

generate_custom_data = cfg["data"]["generate_custom_data"]
port = cfg["serial"]["port"]
baud = cfg["serial"]["baud"]

file_name = cfg["file"]["name"]
out_path = './{}.csv'.format(file_name)

if not generate_custom_data:
    try:
        stm32 = serial.Serial(port, baud)
    except:
        print("Serial not Connected!")
        quit()

    def read_serial(serial, type):
        line = serial.readline()
        if line:
            string = line.decode()
            return type(string)

first = True
while True:
    value = []
    col = []

    time_stmp = datetime.datetime.now()
    time_stmp = time_stmp.strftime("%Y-%m-%d %H:%M:%S")

    for i in range(6):
        if not generate_custom_data:
            value.append( read_serial(stm32, float) )
        else:
            value.append( round(uniform(0.85, 1.0), 2) )

    for i in range(5):
        if not generate_custom_data:
            value.append( read_serial(stm32, float) )
        else:
            value.append( round(uniform(31.2, 31.5), 2) )
    
    col = [time_stmp]
    col.extend(value)

    df = pd.DataFrame({
        'Time': [col[0]],
        'Pressure 1': [col[1]],
        'Pressure 2': [col[2]],
        'Pressure 3': [col[3]],
        'Pressure 4': [col[4]],
        'Pressure 5': [col[5]],
        'Pressure 6': [col[6]],
        'Temperature 1': [col[7]],
        'Temperature 2': [col[8]],
        'Temperature 3': [col[9]],
        'Temperature 4': [col[10]],
        'Temperature 5': [col[11]]
    })

    if first:
        df.to_csv(out_path, index=False, mode='a', header=True)
        first = False
    else:
        df.to_csv(out_path, index=False, mode='a', header=False)

    print(col)
    sleep(1)