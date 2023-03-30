#!./venv/Scripts/python
import os, subprocess, time, yaml

with open("./config.yml", "r") as config:
    cfg = yaml.load(config, Loader=yaml.SafeLoader)
file_name = cfg["file"]["name"]+'.csv'

if os.path.isfile(file_name):
    print (file_name + " already exist!")
    print("Removing old file and create new " + file_name)
    os.remove(file_name)
else:
    print("File doesn't exist, creating " + file_name)

subprocess.Popen('./venv/Scripts/python.exe ./src/getData.py')
time.sleep(1)
subprocess.Popen('./venv/Scripts/python.exe ./src/getGraph.py')

print(".\n.\n.")
print("Don't close this windows while reading data")
print("Press Ctrl+C or Exit to quit")