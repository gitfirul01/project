import sys
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QGridLayout

freq = 0
durasi = 0
interval = 0

class MainWindow(QWidget):
    def _init_(self):
        super()._init_()

        freqLabel = QLabel('Frequenso:', self)
        freqValue = QLabel(freq, self)
        
        durasiLabel = QLabel('Durasi:', self)
        durasiValue = QLabel(durasi, self)

        intervalLabel = QLabel('Interval:', self)
        intervalValue = QLabel(interval, self)
        

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())