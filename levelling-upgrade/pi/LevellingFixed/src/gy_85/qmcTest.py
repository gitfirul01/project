import sys
from i2clibraries.py_qmc5883l import *
from time import sleep

#sensor = QMC5883L(output_range=RNG_8G)
sensor = QMC5883L(1)

while True:
    
    m = sensor.get_magnet()
    print(m)
    sleep(0.2)