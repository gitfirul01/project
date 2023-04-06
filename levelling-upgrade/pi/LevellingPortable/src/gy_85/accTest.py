import sys
from i2clibraries.i2c_adxl345 import *
from time import *

adxl345 = i2c_adxl345(1)

while True:
    accelVal = str(adxl345)
    accelVal = accelVal.split(';')
    
    x = float(accelVal[0])
    
    print(accelVal)
    print(x)
    
    sleep (1)