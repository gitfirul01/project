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

        self.potential_threshold = 120   # batas atas potensi aksi untuk deteksi kontraksi, 120 adalah ujicoba menggunakan otot lengan

        self.countI = 0
        self.countD = 0
        self.levelD = 0

        self.countInterval = 0  # variabel untuk menampung nilai lamanya interval
        self.countDuration = 0  # variabel untuk menampung nilai lamanya durasi

        self.delayms = 150

        self.create_lineChart()

        # self.serial_port = serial.Serial("/dev/ttyUSB0", 9600)

        self.timer_read_data = QTimer()
        self.timer_read_data.setInterval(self.delayms)  # ms
        self.timer_read_data.timeout.connect(self.read_data)
        self.timer_read_data.start()

        self.timer_duration = QTimer()
        self.timer_duration.setInterval(self.delayms) # ms
        self.timer_duration.timeout.connect(self.hitung_durasi)
        self.timer_duration.stop()

        self.timer_interval = QTimer()
        self.timer_interval.setInterval(self.delayms) # ms
        self.timer_interval.timeout.connect(self.hitung_interval)
        self.timer_interval.stop()

        self.timer_send_web = QTimer()
        self.timer_send_web.setInterval(3000) # ms
        self.timer_send_web.timeout.connect(self.kirim_web)
        self.timer_send_web.start()
        
        self.timer_reset = QTimer()
        self.timer_reset.setInterval(600000) # ms
        self.timer_reset.timeout.connect(self.reset_data)
        self.timer_reset.start()

        self.counter = 0


    def create_lineChart(self):
        ## create chart object
        self.chart = QChart()
        self.chart.setAnimationOptions(QChart.SeriesAnimations)
        # self.chart.setTitle("Electrical Signal")
        

        self.chart_view = QChartView(self.chart)
        self.chart_view.setRenderHint(QPainter.Antialiasing)        

        ## create blank widget to store the graphic
        ## we do not do this because we have had chart_widget in .ui file
        ## so we just need to connect that
        # chart_widget = QWidget(self)
        # self.setCentralWidget(chart_widget)

        ## add the widget to chart_layout
        chart_layout = QVBoxLayout(self.chart_widget)
        ## add chart_view widged to chart_layout
        chart_layout.addWidget(self.chart_view)


        self.series = QLineSeries()
        self.chart.addSeries(self.series)

        self.chart.createDefaultAxes()
        self.chart.axisX().setRange(0, 100)
        self.chart.axisY().setRange(0, 200)
        # self.chart.axisX().setTitleText("time")

        self.chart.legend().setVisible(False)
        # self.chart.legend().setAlignment(Qt.AlignBottom)
        # self.chart.legend().markers(self.series)[0].setLabel("Electrical Signal (mV)")


    def read_data(self): 
        x = 0.07*(1000/self.delayms)  ## counter untuk deteksi        (0,07s)
        y = 1*(1000/self.delayms)     ## counter untuk tidak deteksi  (1s)
        z = 2*(1000/self.delayms)     ## counter untuk reset deteksi  (2s)
            
        try:
            # data = self.serial_port.readline().decode().strip() 
            data = randrange(0, 200)
            value = round(float(data), 2)
            self.actPotential = value 

            self.var_frequency.setText(f"{self.frequency}")
            self.var_duration.setText(f"{self.duration}")
            self.var_interval.setText(f"{self.interval}")
            self.var_maxPotent.setText(f"{self.maxPotential}")
            self.var_actPotent.setText(f"{self.actPotential}")

            ## jika terdeteksi sinyal melebihi treshold
            if self.actPotential > self.potential_threshold:
                self.countI += 1
                self.countD = 0
                self.maxPotential = self.actPotential
            ## jika tidak terdeteksi sinyal melebihi treshold
            elif self.actPotential < self.potential_threshold: 
                self.countD += 1 
                if self.actPotential > self.maxPotential:
                    self.maxPotential = self.actPotential
            
            ## jika terdeteksi kontraksi (jumlah sinyal melebihi angka tertentu dan level deteksi = 0)
            if self.countI > x and self.levelD == 0:
                ## update max potential
                if self.actPotential > self.maxPotential:
                    self.maxPotential = self.actPotential
                ## hitung interval
                self.timer_interval.stop()
                self.interval = self.countInterval/(1000/self.delayms)     # convert milisecond of countInterval to second (1 s = 10 * 100 ms)
                self.interval = round(float(self.interval), 2)
                self.frequency += 1
                ## inisialisasi timer durasi kontraksi
                self.countDuration = 0
                self.timer_duration.start()
                ## ubah level deteksi = 1
                self.countD = 0
                self.levelD = 1
            ## jika tidak terdeteksi kontraksi lagi dalam 1 detik
            elif self.countD > y and self.levelD == 1:
                ## update max potential
                if self.actPotential > self.maxPotential:
                    self.maxPotential = self.actPotential
                ## hitung durasi
                self.timer_duration.stop()
                self.duration = self.countDuration/(1000/self.delayms)     # convert milisecond of countDuration to second (1 s = 10 * 100 ms)
                self.duration = round(float(self.duration), 2)
                ## inisialisasi timer interval kontraksi
                self.countInterval = 0
                self.timer_interval.start()
                ## ubah level deteksi = 0
                self.countI = 0
                self.levelD = 0
            ## jika tidak terdeteksi kontraksi lagi dalam 2 detik
            elif self.countD > z:
                if self.actPotential > self.maxPotential: 
                    self.maxPotential = self.actPotential 
                self.countD = 0
                self.countI = 0 

            self.series.append(self.counter, value)
            self.counter += 1
 

            if len(self.series) > 100:
                # self.series.removePoints(0, 1)
                self.chart.axisX().setRange(self.counter - 100, self.counter)
            self.chart.axisY().setRange(0, 200)
 
            self.chart_view.repaint()

        except ValueError:
            pass
    
    def hitung_durasi(self): 
        self.countDuration += 1 

    def hitung_interval(self): 
        self.countInterval += 1

    def reset_data(self):
        self.frequency = 0
        self.duration = 0
        self.interval = 0
        self.countD = 0
        self.countI = 0
        self.levelD = 0
        self.timer_duration.stop()
        self.timer_interval.stop()
    
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
        # self.serial_port.close() 
        super().closeEvent(event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mainWindow()
    window.show()
    window.showMaximized()

    sys.exit(app.exec_())
