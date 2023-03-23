import sys, serial
# import serial.tools.list_ports
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QPainter
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel 
from PyQt5.QtChart import QChart, QChartView, QLineSeries
from PyQt5.uic import loadUi
import requests
  

class MainWindow(QMainWindow):
    def _init_(self):
        super()._init_()
        loadUi('interface.ui', self)

        self.noreg = 1
        self.nama = "nama pasien"
        self.umur = 30
        self.suami = "nama suami"
        self.gpa = "gpa"
        self.humur_hamil = 8

        self.frekuensi = 0
        self.durasi = 0
        self.interval = 0
        self.potensi = 0 
        self.max_potensi = 0 

        #ini adalah batas atas potensi aksi untuk deteksi kontraksi, 120 adalah ujicoba menggunakan otot lengan
        self.tresh_hold = 120 

        self.countU = 0
        self.countD = 0
        self.levelD = 0

        self.countInt = 0
        self.countDur = 0

        self.setWindowTitle("Data Sensor")
        self.setGeometry(100, 100, 800, 480)
        self.setMinimumSize(800,480)
        self.setMaximumSize(800,480)
 
        self.chart = QChart()
        self.chart.legend().hide()
        
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)

        chart_layout = QVBoxLayout(central_widget)
 
        self.chart_view = QChartView(self.chart)
        self.chart_view.setRenderHint(QPainter.Antialiasing)
        chart_layout.addWidget(self.chart_view) 
  
        self.series = QLineSeries()
        self.chart.addSeries(self.series) 

        self.chart.createDefaultAxes()
        self.chart.axisX().setRange(0, 100)
        self.chart.axisY().setRange(0, 200)
 
        self.serial_port = serial.Serial("/dev/ttyUSB0", 9600)
 
        self.timer = QTimer()
        self.timer.setInterval(10)
        self.timer.timeout.connect(self.read_data)
        self.timer.start()

        self.timerD = QTimer()
        self.timerD.setInterval(10)
        self.timerD.timeout.connect(self.hitung_durasi)
        self.timerD.stop()

        self.timerI = QTimer()
        self.timerI.setInterval(10)
        self.timerI.timeout.connect(self.hitung_interval)
        self.timerI.stop()
        
        
        self.timerF = QTimer()
        self.timerF.setInterval(600000)
        self.timerF.timeout.connect(self.reset_data)
        self.timerF.start()

        self.counter = 0

        variable_layout = QHBoxLayout()

        variabel = ["Frekuensi", "Durasi", "Interval", "Max Potensi", "Potensi Aksi", ]
        satuan_variabel = ["kali/10 menit", "detik", "menit", "mV", "mV"]
        value_variabel = [0, 0, 0, 0, 0]
        max_variable = 5

        self.variable_labels = []
        for i in range(max_variable):
            label = QLabel(str(variabel[i]) + ": " + str(value_variabel[i]) + " " + str(satuan_variabel[i]), self) 
            variable_layout.addWidget(label)
            self.variable_labels.append(label)
  
        chart_layout.addLayout(variable_layout)
          

    def read_data(self): 
        x = 7 #counter untuk detek
        y = 100 #counter untuk tidak detek
        z = 200 #counter untuk reset detek

        if self.serial_port is None:
            return 

        data = self.serial_port.readline().decode().strip() 
            
        try: 
            value = float(data) 
            self.potensi = value 

            self.variable_labels[0].setText("Frekuensi: {} kali/10 menit".format(self.frekuensi))
            self.variable_labels[1].setText("Durasi: {} detik".format(self.durasi))
            self.variable_labels[2].setText("Interval: {} detik".format(self.interval))
            self.variable_labels[3].setText("Max Potensi: {} mV".format(self.max_potensi))
            self.variable_labels[4].setText("Potensi Aksi: {} mV".format(self.potensi))

            if self.potensi > self.tresh_hold:
                self.countU += 1
                self.countD = 0 
                self.max_potensi = self.potensi
            elif self.potensi < self.tresh_hold: 
                self.countD += 1 
                if self.potensi > self.max_potensi : self.max_potensi = self.potensi

            if self.countU > x and self.levelD == 0:
                # print("hitung interval dan inisialisasi durasi")
                if self.potensi > self.max_potensi : self.max_potensi = self.potensi

                self.interval = self.countInt/100.0
                self.timerI.stop()
                self.frekuensi += 1
                # print("Interval: " + str(self.countI))
                self.countDur = 0
                self.timerD.start()

                self.countD = 0
                self.levelD = 1
            elif self.countD > y and self.levelD == 1: 
                if self.potensi > self.max_potensi : self.max_potensi = self.potensi

                # print("hitung durasi dan inisialisasi interval")
                self.durasi = self.countDur/100.0
                self.timerD.stop()

                self.kirim_web()
                
                # print("Durasi: " + str(self.countD))
                self.countInt = 0
                self.timerI.start()

                self.countU = 0
                self.levelD = 0
            elif self.countD > z:
                if self.potensi > self.max_potensi : self.max_potensi = self.potensi 

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
        self.frekuensi = 0
        self.durasi = 0
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
                'frequensi': str(self.frekuensi), 'durasi': str(self.durasi),
                'intervaldata': str(self.interval), 'potensi': str(self.max_potensi)}
        
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
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())