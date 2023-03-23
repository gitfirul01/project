import sys, serial, requests

from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel 
from PyQt5.QtGui import QPainter
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtChart import QChart, QChartView, QLineSeries
from PyQt5.uic import loadUi
from threading import Thread, Timer
from time import sleep, time

class mainWindow(QMainWindow):
    def __init__(self):
        super(mainWindow, self).__init__()
        loadUi('interface.ui', self)

        self.create_lineChart()

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

        self.tresh_hold = 120   # batas atas potensi aksi untuk deteksi kontraksi, 120 adalah ujicoba menggunakan otot lengan

        self.countU = 0
        self.countD = 0
        self.levelD = 0

        self.countInt = 0
        self.countDur = 0


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




if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mainWindow()
    window.show()

    sys.exit(app.exec_())