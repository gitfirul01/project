import smbus			#import SMBus module of I2C
from time import sleep          #import

#some MPU6050 Registers and their Address
PWR_MGMT_1   = 0x6B
SMPLRT_DIV   = 0x19
CONFIG       = 0x1A
GYRO_CONFIG  = 0x1B
INT_ENABLE   = 0x38
ACCEL_XOUT_H = 0x3B
ACCEL_YOUT_H = 0x3D
ACCEL_ZOUT_H = 0x3F
GYRO_XOUT_H  = 0x43
GYRO_YOUT_H  = 0x45
GYRO_ZOUT_H  = 0x47


def MPU_Init(bus, device_address):
	#write to sample rate register
	bus.write_byte_data(device_address, SMPLRT_DIV, 7)
	
	#Write to power management register
	bus.write_byte_data(device_address, PWR_MGMT_1, 1)
	
	#Write to Configuration register
	bus.write_byte_data(device_address, CONFIG, 0)
	
	#Write to Gyro configuration register
	bus.write_byte_data(device_address, GYRO_CONFIG, 24)
	
	#Write to interrupt enable register
	bus.write_byte_data(device_address, INT_ENABLE, 1)

def read_raw_data(bus, device_address, addr):
	#Accelero and Gyro value are 16-bit
        high = bus.read_byte_data(device_address, addr)
        low = bus.read_byte_data(device_address, addr+1)
    
        #concatenate higher and lower value
        value = ((high << 8) | low)
        
        #to get signed value from mpu6050
        if(value > 32768):
                value = value - 65536
        return value
