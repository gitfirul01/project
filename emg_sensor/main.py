import sys
import serial
import requests
import csv

# , QWidget, QHBoxLayout, QLabel
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout
from PyQt5.QtGui import QPainter
from PyQt5.QtCore import QTimer  # , Qt
from PyQt5.QtChart import QChart, QChartView, QLineSeries
from PyQt5.uic import loadUi
# from threading import Thread, Timer
# from time import sleep, time

from datetime import datetime
from random import randrange


noreg = 0
nama = "nama pasien"
umur = 0
suami = "nama suami"
gpa = "gpa"
humur_hamil = 0


class main(QMainWindow):
    def __init__(self):
        super(main, self).__init__()
        loadUi('main.ui', self)

        #--- Variabel konfigurasi ---#
        self.DATE = datetime.now().strftime("%d-%B-%Y, %H_%M_%S")
        # self.PATH = '/home/pi/Desktop/Data/'
        self.PATH = './'
        self.csv_fileName = self.PATH + 'EMG-Data-Report-' + self.DATE + '.csv'
        self.max_val = 30
        self.delayms = 100  # periode pembacaan nilai
        self.first = True

        #--- Variabel kontraksi rahim dihitung setiap 10 menit ---#
        self.frekuensi = 0      # berapa kali rahim berkontraksi selama 10 menit
        self.durasi = 0         # berapa detik rahim berkontraksi setiap kali dia berkontraksi
        self.interval = 0       # berapa detik rahim beristirahat (jeda) sebelum berkontraksi kembali
        self.actPotensi = 0     # kekuatan litrik rahim saat berkontraksi
        self.maxPotensi = 0

        self.prev_durasi = 0        # variabel EMA untuk menampung nilai sebelumnya
        self.prev_interval = 0      # variabel EMA untuk menampung nilai sebelumnya
        self.prev_actPotensi = 0    # variabel EMA untuk menampung nilai sebelumnya

        self.potential_threshold = 12.0 # batas atas potensi aksi untuk deteksi kontraksi

        self.countI = 0
        self.countD = 0
        self.levelD = 0

        self.time_count = 0  # variabel untuk menampung nilai lamanya waktu interval dan durasi
        self.counter = 0     # counter siklus pengukuran (x-axis grafik)

        #--- Inisialisasi fungsi ---#
        self.open_serial()
        self.create_lineChart()

        self.btn_red.clicked.connect(self.close)
        self.btn_yellow.clicked.connect(self.showNormal)
        self.btn_green.clicked.connect(self.showFullScreen)
        self.btn_reset.clicked.connect(self.reset_data)
        self.btn_edit.clicked.connect(self.open_edit_dialog)
        self.dialogs = list()

        self.timer_read_data = QTimer()
        self.timer_read_data.setInterval(self.delayms)  # ms
        self.timer_read_data.timeout.connect(self.read_data)
        self.timer_read_data.start()

        self.timer_counter = QTimer()
        self.timer_counter.setInterval(self.delayms)  # ms
        self.timer_counter.timeout.connect(self.time_counter)
        self.timer_counter.stop()

        self.timer_send_web = QTimer()
        self.timer_send_web.setInterval(3000)  # ms
        self.timer_send_web.timeout.connect(self.save_data)
        self.timer_send_web.start()

        self.timer_reset = QTimer()
        self.timer_reset.setInterval(600000)  # ms
        self.timer_reset.timeout.connect(self.reset_data)
        self.timer_reset.start()


    def open_serial(self):
        try:
            self.serial_port = serial.Serial("/dev/ttyUSB0", 9600)
            print("Berhasil membuka serial port")
            self.console.setText((f"Berhasil membuka serial port"))
        except:
            print("Gagal membuka serial port, generate data random")
            self.console.setText((f"Gagal membuka serial port, generate data random"))

    def open_edit_dialog(self):
        dialog = editData()
        self.dialogs.append(dialog)
        # dialog.show()
        dialog.showFullScreen()

    def create_lineChart(self):
        # create chart object
        self.chart = QChart()
        self.chart.setAnimationOptions(QChart.SeriesAnimations)
        # self.chart.setTitle("Electrical Signal")

        self.chart_view = QChartView(self.chart)
        self.chart_view.setRenderHint(QPainter.Antialiasing)

        # create blank widget to store the graphic
        # we do not do this because we have had chart_widget in .ui file
        # so we just need to connect that
        # chart_widget = QWidget(self)
        # self.setCentralWidget(chart_widget)

        # add the widget to chart_layout
        chart_layout = QVBoxLayout(self.chart_widget)
        # add chart_view widged to chart_layout
        chart_layout.addWidget(self.chart_view)

        self.series = QLineSeries()
        self.chart.addSeries(self.series)

        self.chart.createDefaultAxes()
        self.chart.axisX().setRange(0, 100)
        # self.chart.axisY().setRange(0, 5000)
        self.chart.axisY().setRange(0, self.max_val)
        # self.chart.axisX().setTitleText("time")

        self.chart.legend().setVisible(False)
        # self.chart.legend().setAlignment(Qt.AlignBottom)
        # self.chart.legend().markers(self.series)[0].setLabel("Electrical Signal (mV)")

    def read_data(self):
        x = 0.07*(1000/self.delayms)  # counter untuk deteksi        (0,07s)
        y = 1*(1000/self.delayms)     # counter untuk tidak deteksi  (1s)
        z = 2*(1000/self.delayms)     # counter untuk reset deteksi  (2s)

        try:
            try:
                data = float(self.serial_port.readline().decode().strip())
            except:
                data = randrange(0, 20)
                
            # filter nilai potensi aksi
            self.actPotensi = self.ema(data, self.prev_actPotensi, 0.5)
            self.actPotensi = round(float(self.actPotensi), 2)
            self.prev_actPotensi = self.actPotensi

            self.series.append(self.counter, self.actPotensi)

            # jika terdeteksi sinyal melebihi treshold
            if self.actPotensi > self.potential_threshold:
                self.countI += 1
                self.countD = 0
                self.maxPotensi = self.actPotensi
            # jika tidak terdeteksi sinyal melebihi treshold
            elif self.actPotensi < self.potential_threshold:
                self.countD += 1
                if self.actPotensi > self.maxPotensi:
                    self.maxPotensi = self.actPotensi

            # jika terdeteksi kontraksi (jumlah sinyal melebihi angka tertentu dan level deteksi = 0)
            if self.countI > x and self.levelD == 0:
                self.frekuensi += 1
                # update max potential
                if self.actPotensi > self.maxPotensi:
                    self.maxPotensi = self.actPotensi
                # hitung interval
                self.timer_counter.stop()
                # convert milisecond of time_count to second (1 s = 10 * 100 ms)
                self.interval = round(float(
                    self.ema(self.time_count/(1000/self.delayms),
                             self.prev_interval, 0.5)),
                    2)
                self.prev_interval = self.interval
                # inisialisasi timer durasi kontraksi
                self.time_count = 0
                self.timer_counter.start()
                # ubah level deteksi = 1
                self.countD = 0
                self.levelD = 1
            # jika tidak terdeteksi kontraksi lagi dalam 1 detik
            elif self.countD > y and self.levelD == 1:
                # update max potential
                if self.actPotensi > self.maxPotensi:
                    self.maxPotensi = self.actPotensi
                # hitung durasi
                self.timer_counter.stop()
                # convert milisecond of time_count to second (1 s = 10 * 100 ms)
                self.durasi = round(float(
                    self.ema(self.time_count/(1000/self.delayms),
                             self.prev_interval, 0.5)),
                    2)
                self.prev_durasi = self.durasi
                # inisialisasi timer interval kontraksi
                self.time_count = 0
                self.timer_counter.start()
                # ubah level deteksi = 0
                self.countI = 0
                self.levelD = 0
            # jika tidak terdeteksi kontraksi lagi dalam 2 detik
            elif self.countD > z:
                if self.actPotensi > self.maxPotensi:
                    self.maxPotensi = self.actPotensi
                self.countD = 0
                self.countI = 0

            # tampilkan nilai pada GUI
            self.var_frequency.setText(f"{self.frekuensi}")
            self.var_duration.setText(f"{self.durasi}")
            self.var_interval.setText(f"{self.interval}")
            self.var_maxPotent.setText(f"{self.maxPotensi}")
            self.var_actPotent.setText(f"{self.actPotensi}")

            global noreg, nama, umur
            self.label_noreg.setText(f"{noreg}")
            self.label_nama.setText(f"{nama}")
            self.label_umur.setText(f"{umur}")

            self.counter += 1

            if len(self.series) > 100:
                # self.series.removePoints(0, 1)
                self.chart.axisX().setRange(self.counter - 100, self.counter)
            if self.actPotensi > self.max_val:
                self.max_val = self.actPotensi
                self.chart.axisY().setRange(0, 3*self.max_val)
            # self.chart.axisY().setRange(0, 5000)

            self.chart_view.repaint()

        except ValueError:
            pass

    def time_counter(self):
        self.time_count += 1

    def reset_data(self):
        print("Data reset")
        self.console.setText((f"Data reset"))

        self.frekuensi = 0
        self.durasi = 0
        self.interval = 0
        self.countD = 0
        self.countI = 0
        self.levelD = 0

        self.timer_counter.stop()

    def save_data(self):
        global noreg, nama, umur, suami, gpa, humur_hamil
        self.csv_export()
        try:
            url = 'https://uterus.sogydevelop.com/datakirim.php'
            data = {'noreg': str(noreg), 'nama': str(nama), 'umur': str(umur),
                    'suami': suami, 'gpa': self. gpa, 'umurkehamilan': str(humur_hamil),
                    'frequensi': str(self.frekuensi), 'durasi': str(self.durasi),
                    'intervaldata': str(self.interval), 'potensi': str(self.maxPotensi)}

            response = requests.post(url, data=data, auth=('admin12345', '12345678'), timeout=2)

            if response.status_code == 200:
                print('Data berhasil dikirim ke server')
                self.console.setText((f"Data berhasil dikirim ke server"))
            else:
                print('Terjadi kesalahan dalam mengirim data ke server')
                self.console.setText(
                    (f"Terjadi kesalahan dalam mengirim data ke server"))
        except:
            print('Terjadi kesalahan dalam mengirim data ke server')
            self.console.setText(
                (f"Terjadi kesalahan dalam mengirim data ke server"))

    def closeEvent(self, event):
        try:
            self.serial_port.close()
        except:
            pass
        # super().closeEvent(event)

    def ema(self, data_now, data_prev, alfa):
        # https://www.investopedia.com/terms/m/movingaverage.asp
        return (data_now*alfa) + (data_prev*(1-alfa))

    def csv_export(self):
        global noreg, nama, umur, suami, gpa, humur_hamil
        try:
            with open(self.csv_fileName, newline='') as csvfile:
                csvfile.close()
            self.first = False
        except:
            self.first = True

        with open(self.csv_fileName, 'a', newline='') as csvfile:
            timestamp = datetime.now().strftime("%d-%B-%Y %H:%M:%S")
            field_names = ['Time', 'No Reg', 'Nama', 'Umur', 'Suami', 'GPA',
                           'Umur Kehamilan', 'Frekuensi', 'Durasi', 'Interval', 'Potensi']
            writer = csv.DictWriter(csvfile, fieldnames=field_names)

            if self.first == True:
                writer.writeheader()
            writer.writerow({'Time': timestamp, 'No Reg': noreg, 'Nama': nama, 'Umur': umur, 'Suami': suami, 'GPA': gpa, 'Umur Kehamilan': humur_hamil,
                             'Frekuensi': self.frekuensi, 'Durasi': self.durasi, 'Interval': self.interval, 'Potensi': self.maxPotensi})
            csvfile.close()


class editData(QMainWindow):
    def __init__(self):
        super(editData, self).__init__()
        loadUi('widget.ui', self)

        self.btn_done.clicked.connect(self.close)

        self.in_noreg.textChanged.connect(self.noregChanged)
        self.in_nama.textChanged.connect(self.namaChanged)
        self.in_umur.textChanged.connect(self.umurChanged)
        self.in_suami.textChanged.connect(self.suamiChanged)
        self.in_gpa.textChanged.connect(self.gpaChanged)
        self.in_umurkehamilan.textChanged.connect(self.umurkehamilanChanged)

    def noregChanged(self, text):
        global noreg
        noreg = text
    def namaChanged(self, text):
        global nama
        nama = text
    def umurChanged(self, text):
        global umur
        umur = text
    def suamiChanged(self, text):
        global suami
        suami = text
    def gpaChanged(self, text):
        global gpa
        gpa = text
    def umurkehamilanChanged(self, text):
        global humur_hamil
        humur_hamil = text


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = main()
    # window.show()
    window.showFullScreen()

    sys.exit(app.exec_())
