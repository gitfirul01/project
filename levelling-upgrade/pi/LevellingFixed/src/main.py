#!/usr/bin/python3

import sys, math, pandas as pd

from threading import Thread
from time import sleep
from PyQt5.uic import loadUi
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QDialog, QApplication, QMessageBox
from PyQt5 import QtCore, QtGui, QtWidgets

## local lib
from export_file import *
from ultrasonic import  *
from mpu6050ac import *

ultrasonic = DFRobot_A02_Distance("/dev/ttyUSB0", 9600)
dis_min = 30    # Minimum ranging threshold: 0mm
dis_max = 7500  # Highest ranging threshold: 7500mm
ultrasonic.set_dis_range(dis_min, dis_max)

accel_bus = smbus.SMBus(1)
device_address = 0x68
MPU_Init(accel_bus, device_address)

## global variable
panjang = 0.00
lebar = 0.00
tinggi = 0.00
diameter = 0.00

distance = 0.00
volumeBefore = 0.00
volumeAfter = 0.00
deltaVolume = 0.00
data = []

pitch = 0.00
roll = 0.00
yaw = 0.00

start = False

DIR = '/home/pi/LevellingFixed/'

class screen1(QDialog):
    def __init__(self):
        super(screen1, self).__init__()
        loadUi(DIR+"UI/screen1.ui",self)
        self.setWindowTitle("Window Title")
        self.startButton.clicked.connect(self.gotoscreen2)
        self.exitButton.clicked.connect(self.exit)

    def gotoscreen2(self):
        widget.setCurrentIndex(widget.currentIndex()+1)
        
    def exit(self):
        sys.exit()

class screen2(QDialog):
    def __init__(self):
        super(screen2, self).__init__()
        loadUi(DIR+"UI/screen2.ui",self)
        self.homeButton.clicked.connect(self.back_action)
        self.saveButton.clicked.connect(self.save_action)
        self.powerButton.clicked.connect(self.power_action)
        
        self.tableWidget.setColumnWidth(0, 125)
        self.tableWidget.setColumnWidth(1, 125)
        self.tableWidget.setColumnWidth(2, 125)
        self.tableWidget.setColumnWidth(3, 125)

        self.tableWidget.verticalScrollBar().setVisible(False)
        self.tableWidget.horizontalScrollBar().setVisible(False)

        ## timer for update threading data
        self.timer1 = QtCore.QTimer()
        self.timer1.timeout.connect(self.load_imu)
        self.timer1.start(300)
        ## OR
        # self.timer1.setInterval(300) # in milliseconds
        # self.timer1.start()

        self.timer2 = QtCore.QTimer()
        self.timer2.timeout.connect(self.load_volume)
        self.timer2.start(60000)

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-1)
    
    def save_action(self):
        read_file = pd.read_csv(PATH + TEMP + file_name + '.csv')
        read_file.to_excel(PATH + file_name + '.xlsx', index = None, header=True)
        
        msg = QMessageBox()
        msg.setText("Data Saved")
        msg.setIcon(QMessageBox.Information)
        x = msg.exec()

    def power_action(self):
        if self.powerButton.isChecked():
            self.powerButton.setStyleSheet(\
                "QPushButton#powerButton:checked {color: white; background-color: rgba(5, 125, 5, 255); }")
            self.power_status.setText(QtCore.QCoreApplication.translate("Dialog", \
                "<html><head/><body><p><span style=\" font-weight:600;\">ON</span></p></body></html>"))
        else:
            self.powerButton.setStyleSheet(\
                "QPushButton#powerButton {color: white; background-color: rgba(235, 30, 40, 255); }")
            self.power_status.setText(QtCore.QCoreApplication.translate("Dialog", \
                "<html><head/><body><p><span style=\" font-weight:600;\">OFF</span></p></body></html>"))
    
    def load_imu(self):
        global roll, pitch, yaw
        if self.powerButton.isChecked():    
            self.xVal.setText(f"{roll}")
            self.yVal.setText(f"{pitch}")
            self.zVal.setText(f"{yaw}")

    def load_volume(self):
        global data, volumeBefore, volumeAfter, deltaVolume
        if start:
            date = datetime.now().strftime("%d/%b/%Y")
            time = datetime.now().strftime("%H:%M:%S")
            ## save data to temp csv file
            dataRaw = {"Tanggal":date, "Waktu":time, "Volume":volumeAfter, "Selisih":deltaVolume}
            save_csv(dataRaw)
            ##
            count = len(data)
            if count >= 59:
                data.pop(0)
            data.append(dataRaw)
            volumeBefore = volumeAfter
            ## 
            row = 0
            self.tableWidget.setRowCount(len(data))
            for i in range( len(data) - 1, -1, -1):         # iterate in reverse order
                def tab_assign(index, data):
                    item = QtWidgets.QTableWidgetItem(data)
                    item.setTextAlignment(QtCore.Qt.AlignCenter)
                    if index%2 == 0:
                        item.setBackground(QtGui.QColor(250,250,250))
                    return item
                self.tableWidget.setItem(row, 0, tab_assign(i, data[i]["Tanggal"]))
                self.tableWidget.setItem(row, 1, tab_assign(i, data[i]["Waktu"]))
                self.tableWidget.setItem(row, 2, tab_assign(i, str(data[i]["Volume"])))
                self.tableWidget.setItem(row, 3, tab_assign(i, str(data[i]["Selisih"])))
                row = row+1

## thread function
def volume_read():
    global volumeBefore, volumeAfter, deltaVolume
    while True:
        distance = ultrasonic.get_distance()
        volumeAfter = panjang*lebar*abs(tinggi-distance) # volume in mL
        deltaVolume = abs(volumeBefore - volumeAfter)

        if start == True:
            save_realtime_csv(volumeAfter)
        sleep(5)

def imu_read():
    global pitch, roll, yaw
    while True:
        ## Read Accelerometer raw value
        acc_x = read_raw_data(accel_bus, device_address, ACCEL_XOUT_H)
        acc_y = read_raw_data(accel_bus, device_address, ACCEL_YOUT_H)
        acc_z = read_raw_data(accel_bus, device_address, ACCEL_ZOUT_H)
        ## Full scale range +/- 250 degree/C as per sensitivity scale factor
        Ax = acc_x/16384.0
        Ay = acc_y/16384.0
        Az = acc_z/16384.0
        ##
        pitch = math.atan2(Ax, math.sqrt(Ay*Ay + Az*Az)) *180/math.pi
        roll = math.atan2(Ay, Az) *180/math.pi
        yaw = math.atan2(Az, math.sqrt(Ax*Ax + Az*Az)) *180/math.pi
        sleep(0.3)
        

## main program
if __name__ == "__main__":
    app = QApplication(sys.argv)

    ## threading
    imuHandler = Thread(target=imu_read)
    imuHandler.setDaemon(True)
    imuHandler.start()

    volumeHandler = Thread(target=volume_read)
    volumeHandler.setDaemon(True)
    volumeHandler.start()
    ##
    widget = QtWidgets.QStackedWidget()
    widget.addWidget(screen1())
    widget.addWidget(screen2())

    widget.setFixedWidth(1024)
    widget.setFixedHeight(600)
    widget.showFullScreen()
    widget.show()
 
    sys.exit(app.exec_())