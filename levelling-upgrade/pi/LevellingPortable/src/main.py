#!/usr/bin/python3

import sys, math

from threading import Thread #, Timer
from time import sleep
from PyQt5.uic import loadUi
from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt #, reset
from PyQt5.QtWidgets import QDialog, QApplication, QMessageBox
from PyQt5 import QtCore, QtGui, QtWidgets

## local lib
from export_file import *
from ultrasonic import  *
# from mpu6050ac import *
from gy_85ac import *

ultrasonic = DFRobot_A02_Distance("/dev/ttyUSB0", 9600)
dis_min = 30    # Minimum ranging threshold: 0mm
dis_max = 7500  # Highest ranging threshold: 7500mm
ultrasonic.set_dis_range(dis_min, dis_max)

## if using GY85 or adxl345
accelerometer = i2c_adxl345(1)
## if using GY87 or mpu6050
# accel_bus = smbus.SMBus(1)
# device_address = 0x68
# MPU_Init(accel_bus, device_address)

## global variable
operator = ''
driver = ''
nopol = ''
tangki = ''

panjang = 0.00
lebar = 0.00
tinggi = 0.00
diameter = 0.00
deltaVolume = 0.00

tinggiBefore = 0.00
tinggiAfter = 0.00
volumeBefore = 0.00
volumeAfter = 0.00

accelStr = ''
accelVal = []
pitch = 0.00
roll = 0.00
yaw = 0.00

distance = 0.00

DIR = '/home/pi/LevellingPortable/'

def handleVisibleChanged():
    if not QtGui.QGuiApplication.inputMethod().isVisible():
        return
    for w in QtGui.QGuiApplication.allWindows():
        if w.metaObject().className() == "QtVirtualKeyboard::InputView":
            keyboard = w.findChild(QtCore.QObject, "keyboard")
            if keyboard is not None:
                r = w.geometry()
                r.moveTop(keyboard.property("y"))
                w.setMask(QtGui.QRegion(r))
                return

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
        self.backButton.clicked.connect(self.back_action)
        self.nextButton.clicked.connect(self.next_action)
        
        self.operatorText.textChanged.connect(self.operatorChanged)
        self.driverText.textChanged.connect(self.driverChanged)
        self.nopolText.textChanged.connect(self.nopolChanged)

    def operatorChanged(self, text):
        global operator
        operator = text
        
    def driverChanged(self, text):
        global driver
        driver = text
    
    def nopolChanged(self, text):
        global nopol
        nopol = text

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-1)

    def next_action(self):
        widget.setCurrentIndex(widget.currentIndex()+1)

class screen3(QDialog):
    def __init__(self):
        super(screen3, self).__init__()
        loadUi(DIR+"UI/screen3.ui",self)
        self.backButton.clicked.connect(self.back_action)
        self.tabungButton.clicked.connect(self.tabung_action)
        self.kubusButton.clicked.connect(self.kubus_action)

    def tabung_action(self):
        global tangki
        tangki = 'tabung'
        widget.setCurrentIndex(widget.currentIndex()+2)

    def kubus_action(self):
        global tangki
        tangki = 'kubus'
        widget.setCurrentIndex(widget.currentIndex()+1)

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-1)

class screen4(QDialog):
    def __init__(self):
        super(screen4, self).__init__()
        loadUi(DIR+"UI/screen4.ui",self)
        self.backButton.clicked.connect(self.back_action)
        self.nextButton.clicked.connect(self.next_action)
        self.panjangText.textChanged.connect(self.panjangChanged)
        self.lebarText.textChanged.connect(self.lebarChanged)

    def panjangChanged(self, text):
        global panjang
        try:
            panjang = int(text)
        except:
            panjang = 'masukan angka'

    def lebarChanged(self, text):
        global lebar
        try:
            lebar = int(text)
        except:
            lebar = 'masukan angka'

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-1)

    def next_action(self):
        widget.setCurrentIndex(widget.currentIndex()+2)

class screen5(QDialog):
    def __init__(self):
        super(screen5, self).__init__()
        loadUi(DIR+"UI/screen5.ui",self)
        self.backButton.clicked.connect(self.back_action)
        self.nextButton.clicked.connect(self.next_action)
        self.diameterText.textChanged.connect(self.diameterChanged)

    def diameterChanged(self, text):
        global diameter
        try:
            diameter = int(text)
        except:
            diameter = 'masukan angka'

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-2)

    def next_action(self):
        widget.setCurrentIndex(widget.currentIndex()+1)

class screen6(QDialog):
    def __init__(self):
        super(screen6, self).__init__()
        loadUi(DIR+"UI/screen6.ui",self)
        
        self.backButton.clicked.connect(self.back_action)
        self.nextButton.clicked.connect(self.next_action)
        # self.beforeButton.clicked.connect(self.dataBefore)
        # self.afterButton.clicked.connect(self.dataAfter)

        ## timer for update threading data
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.thread_update)
        self.timer.start(300)

    def thread_update(self):
        global pitch, yaw, roll, distance, tinggiBefore, tinggiAfter
        
        self.xValue.setText(f"{pitch:.2f}")
        self.yValue.setText(f"{roll:.2f}")
        self.zValue.setText(f"{yaw:.2f}")
        
        if self.beforeButton.isChecked():
            self.beforeButton.setStyleSheet("QPushButton#powerButton:checked {color: white; background-color: rgba(252, 234, 29, 255); }")
            tinggiBefore = distance
            self.beforeValue.setText(f"{distance} mm")
            if ((pitch > -0.5) and (pitch < 0.5)) and ((roll > -0.5) and (roll < 0.5)):
                self.beforeButton.setChecked(False)
        else:
            self.beforeButton.setStyleSheet("QPushButton#powerButton:checked {color: white; background-color: rgba(252, 234, 29, 255); }")

        if self.afterButton.isChecked():
            self.afterButton.setStyleSheet("QPushButton#powerButton:checked {color: white; background-color: rgba(252, 234, 29, 255); }")
            tinggiAfter = distance
            self.afterValue.setText(f"{distance} mm")
            if ((pitch > -0.5) and (pitch < 0.5)) and ((roll > -0.5) and (roll < 0.5)):
                self.afterButton.setChecked(False)
        else:
            self.afterButton.setStyleSheet("QPushButton#powerButton:checked {color: white; background-color: rgba(252, 234, 29, 255); }")

    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-3)

    def next_action(self):
        widget.setCurrentIndex(widget.currentIndex()+1)

class screen7(QDialog):
    def __init__(self):
        super(screen7, self).__init__()
        loadUi(DIR+"UI/screen7.ui",self)
        self.homeButton.clicked.connect(self.gotoscreen1)
        self.saveButton.clicked.connect(self.save)
        self.refresh_btn.clicked.connect(self.show_results)
        self.backButton.clicked.connect(self.back_action)
        
    def show_results(self):
        global panjang, lebar, tinggi, diameter, \
               deltaVolume, volumeBefore, volumeAfter, \
               tinggiBefore, tinggiAfter, tangki
               
        if (tangki=='kubus'):
            volumeBefore = panjang *lebar *abs(tinggi-tinggiBefore) *0.000001
            volumeAfter = panjang *lebar *abs(tinggi-tinggiAfter) *0.000001
        elif (tangki=='tabung'):
            volumeBefore = math.pi *( 0.5 *diameter )**2 *abs(tinggi-tinggiBefore) *0.000001
            volumeAfter = math.pi *( 0.5 *diameter )**2 *abs(tinggi-tinggiAfter) *0.000001

        deltaVolume = abs(volumeBefore - volumeAfter)
        self.VolumeView.setAlignment(Qt.AlignLeft)
        self.VolumeView.setText(f'{deltaVolume:.2f} L')

    def gotoscreen1(self):
        sys.stdout.flush()
        os.execl(sys.executable, 'python3', __file__, *sys.argv[1:])

        # global operator, driver, nopol, panjang, lebar, tinggiAfter, tinggiBefore, volumeAfter, volumeBefore, tangki, diameter
        # operator = ''
        # driver = ''
        # nopol = ''
        # panjang = 0.00
        # lebar = 0.00
        # tinggiBefore = 0.00
        # tinggiAfter = 0.00
        # volumeBefore = 0.00
        # volumeAfter = 0.00
        # diameter = 0.00
        # tangki = ''

        # widget.setCurrentIndex(0)

    def save(self):
        global operator, driver, nopol, volumeBefore, volumeAfter, deltaVolume
        create_analytics_report(operator, driver, nopol, volumeBefore, volumeAfter, deltaVolume)
        
        msg = QMessageBox()
        msg.setText("Data Saved")
        msg.setIcon(QMessageBox.Information)
        x = msg.exec()
        
    def back_action(self):
        widget.setCurrentIndex(widget.currentIndex()-1)


## additional function
def sensor_read():
    global accelStr, accelVal, pitch, roll, yaw, distance
    while True:
        ## IF USING ADXL345
        accelStr = str(accelerometer)
        accelVal = accelStr.split(';')
        Ax = float(accelVal[0])
        Ay = float(accelVal[1])
        Az = float(accelVal[2])

        ## IF USING MPU6050
        # #Read Accelerometer raw value
        # acc_x = read_raw_data(accel_bus, device_address, ACCEL_XOUT_H)
        # acc_y = read_raw_data(accel_bus, device_address, ACCEL_YOUT_H)
        # acc_z = read_raw_data(accel_bus, device_address, ACCEL_ZOUT_H)
        # #Full scale range +/- 250 degree/C as per sensitivity scale factor
        # Ax = acc_x/16384.0
        # Ay = acc_y/16384.0
        # Az = acc_z/16384.0
        
        ###
        pitch = math.atan2(Ax, math.sqrt(Ay*Ay + Az*Az)) *180/math.pi
        roll = math.atan2(Ay, Az) *180/math.pi
        yaw = math.atan2(Az, math.sqrt(Ax*Ax + Az*Az)) *180/math.pi

        distance = ultrasonic.get_distance()

        sleep(0.3)

        

## main program
if __name__ == "__main__":
    os.environ["QT_IM_MODULE"] = "qtvirtualkeyboard"

    app = QApplication(sys.argv)
    QtGui.QGuiApplication.inputMethod().visibleChanged.connect(handleVisibleChanged)

    ## threading
    writeHandler = Thread(target=sensor_read)
    writeHandler.setDaemon(True)
    writeHandler.start()

    widget = QtWidgets.QStackedWidget()

    widget.addWidget(screen1())
    widget.addWidget(screen2())
    widget.addWidget(screen3())
    widget.addWidget(screen4())
    widget.addWidget(screen5())
    widget.addWidget(screen6())
    widget.addWidget(screen7())

    widget.setFixedWidth(1024)
    widget.setFixedHeight(600)
    widget.showFullScreen()
    widget.show()
 
    sys.exit(app.exec_())