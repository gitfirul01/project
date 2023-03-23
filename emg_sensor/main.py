import sys, serial, requests

from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel 
from PyQt5.QtGui import QPainter
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtChart import QChart, QChartView, QLineSeries
from PyQt5.uic import loadUi
from threading import Thread, Timer
from time import sleep, time

from random import randrange

class mainWindow(QMainWindow):
    def __init__(self):
        super(mainWindow, self).__init__()
        loadUi('interface.ui', self)

        self.noreg = 1
        self.nama = "nama pasien"
        self.umur = 30
        self.suami = "nama suami"
        self.gpa = "gpa"
        self.humur_hamil = 8

        ## Kontraksi rahim dihitung setiap 10 menit
        self.frequency = 0      # berapa kali rahim berkontraksi selama 10 menit
        self.duration = 0       # berapa detik rahim berkontraksi setiap kali dia berkontraksi
        self.interval = 0       # berapa detik rahim beristirahat (jeda) sebelum berkontraksi kembali    
        self.actPotential = 0   # kekuatan litrik rahim saat berkontraksi
        self.maxPotential = 0 

        self.tresh_hold = 120   # batas atas potensi aksi untuk deteksi kontraksi, 120 adalah ujicoba menggunakan otot lengan

        self.countU = 0
        self.countD = 0
        self.levelD = 0

        self.countInt = 0
        self.countDur = 0

        self.create_lineChart()

        # self.serial_port = serial.Serial("/dev/ttyUSB0", 9600)

        self.timer = QTimer()
        self.timer.setInterval(10)  # ms
        self.timer.timeout.connect(self.read_data)
        self.timer.start()

        self.timerD = QTimer()
        self.timerD.setInterval(10) # ms
        self.timerD.timeout.connect(self.hitung_durasi)
        self.timerD.stop()

        self.timerI = QTimer()
        self.timerI.setInterval(10) # ms
        self.timerI.timeout.connect(self.hitung_interval)
        self.timerI.stop()
        
        
        self.timerF = QTimer()
        self.timerF.setInterval(600000) # ms
        self.timerF.timeout.connect(self.reset_data)
        self.timerF.start()

        self.counter = 0


    def create_lineChart(self):
        # create chart object
        self.chart = QChart()
        self.chart.setAnimationOptions(QChart.SeriesAnimations)
        # self.chart.setTitle("Electrical Signal")
        self.chart.legend().setVisible(True)
        self.chart.legend().setAlignment(Qt.AlignBottom)


        self.chart_view = QChartView(self.chart)
        self.chart_view.setRenderHint(QPainter.Antialiasing)        

        # # create blank widget to store the graphic
        # # we do not do this because we have had chart_widget in .ui file
        # # so we just need to connect that
        # chart_widget = QWidget(self)
        # self.setCentralWidget(chart_widget)

        # add the widget to chart_layout
        chart_layout = QVBoxLayout(self.chart_widget)
        # add chart_view widged to chart_layout
        chart_layout.addWidget(self.chart_view)


        self.series = QLineSeries()
        # update series ?
        self.chart.addSeries(self.series)

        self.chart.createDefaultAxes()
        self.chart.axisX().setRange(0, 100)
        self.chart.axisY().setRange(0, 200)


    def read_data(self): 
        x = 7 # counter untuk deteksi
        y = 100 # counter untuk tidak deteksi
        z = 200 # counter untuk reset deteksi

        # data = self.serial_port.readline().decode().strip() 
        data = randrange(0, 150)
            
        try: 
            value = round(float(data), 2)
            self.actPotential = value 

            self.var_frequency.setText(f"{self.frequency}")
            self.var_duration.setText(f"{self.duration}")
            self.var_interval.setText(f"{self.interval}")
            self.var_maxPotent.setText(f"{self.maxPotential}")
            self.var_actPotent.setText(f"{self.actPotential}")

            if self.actPotential > self.tresh_hold:
                self.countU += 1
                self.countD = 0 
                self.maxPotential = self.actPotential
            elif self.actPotential < self.tresh_hold: 
                self.countD += 1 
                if self.actPotential > self.maxPotential : self.maxPotential = self.actPotential

            if self.countU > x and self.levelD == 0:
                # print("hitung interval dan inisialisasi durasi")
                if self.actPotential > self.maxPotential : self.maxPotential = self.actPotential

                self.interval = self.countInt/100.0
                self.timerI.stop()
                self.frequency += 1
                # print("Interval: " + str(self.countI))
                self.countDur = 0
                self.timerD.start()

                self.countD = 0
                self.levelD = 1
            elif self.countD > y and self.levelD == 1: 
                if self.actPotential > self.maxPotential : self.maxPotential = self.actPotential

                # print("hitung durasi dan inisialisasi interval")
                self.duration = self.countDur/100.0
                self.timerD.stop()

                self.kirim_web()
                
                # print("Durasi: " + str(self.countD))
                self.countInt = 0
                self.timerI.start()

                self.countU = 0
                self.levelD = 0
            elif self.countD > z:
                if self.actPotential > self.maxPotential : self.maxPotential = self.actPotential 

                self.countD = 0
                self.countU = 0 
 
            self.series.append(self.counter, value)
            self.counter += 1
 
            if len(self.series) > 100:
                self.series.removePoints(0, 1)
 
            self.chart.axisX().setRange(self.counter - 100, self.counter)
            self.chart.axisY().setRange(0, 200)
 
            self.chart_view.repaint()

        except ValueError:
            pass
    
    def hitung_durasi(self): 
        self.countDur += 1 

    def hitung_interval(self): 
        self.countInt += 1

    def reset_data(self):
        self.frequency = 0
        self.duration = 0
        self.interval = 0
        self.countD = 0
        self.countU = 0
        self.levelD = 0
        self.timerD.stop()
        self.timerI.stop()
    
    def kirim_web(self):
        url = 'https://uterus.sogydevelop.com/datakirim.php'
 

        data = {'noreg': str(self.noreg), 'nama': str(self.nama),'umur' : str(self.umur), 
                'suami': self.suami,'gpa':self. gpa, 'umurkehamilan' : str(self.humur_hamil), 
                'frequensi': str(self.frequency), 'durasi': str(self.duration),
                'intervaldata': str(self.interval), 'potensi': str(self.maxPotential)}
        
        response = requests.post(url, data=data, auth=('admin12345', '12345678'))

        if response.status_code == 200: 
            print('Data berhasil dikirim ke server')
        else: 
            print('Terjadi kesalahan dalam mengirim data ke server')

    def closeEvent(self, event): 
        self.serial_port.close() 
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mainWindow()
    window.show()

    sys.exit(app.exec_())
