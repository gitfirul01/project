#!/usr/bin/python3

from datetime import datetime
import csv

PATH = '/home/pi/Desktop/DataReport/'
TEMP = '.temp/'

file_name = 'data_report'
realtime_file_name = 'realtime_data_report'

def check_file_exist(filename):
    try:
        with open(filename, newline='') as csvfile:
            csvfile.close()
        return False
    except:
        return True

def save_csv(item):
    first = check_file_exist(PATH + TEMP + file_name + '.csv')
    with open(PATH + TEMP + file_name + '.csv', 'a', newline='') as csvfile:
        field_names = ['Tanggal', 'Waktu', 'Volume', 'Selisih']
        writer = csv.DictWriter(csvfile, fieldnames=field_names)
        if first == True:
            writer.writeheader()
        writer.writerow(item)
        csvfile.close()

def save_realtime_csv(volume):
    first = check_file_exist(PATH + realtime_file_name + '.csv')
    with open(PATH + realtime_file_name + '.csv', 'a', newline='') as csvfile:
        date = datetime.now().strftime("%d/%b/%Y")
        time = datetime.now().strftime("%H:%M:%S")

        field_names = ['Tanggal', "Waktu", 'Volume']
        writer = csv.DictWriter(csvfile, fieldnames=field_names)
        if first == True:
            writer.writeheader()
        writer.writerow({"Tanggal":date, "Waktu":time, 'Volume':volume})
        csvfile.close()


