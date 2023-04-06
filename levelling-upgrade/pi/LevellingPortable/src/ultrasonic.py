import serial
import time

class DFRobot_A02_Distance:

  ''' Board status '''
  STA_OK = 0x00
  STA_ERR_CHECKSUM = 0x01
  STA_ERR_SERIAL = 0x02
  STA_ERR_CHECK_OUT_LIMIT = 0x03
  STA_ERR_CHECK_LOW_LIMIT = 0x04
  STA_ERR_DATA = 0x05

  ''' last operate status, users can use this variable to determine the result of a function call. '''
  last_operate_status = STA_OK

  ''' variable '''
  distance = 0

  '''Maximum range'''
  distance_max = 7500
  distance_min = 30
  range_max = 7200

  def __init__(self, port, baud):
    self.port = port
    self.baud = baud
    # self.ser = serial.Serial("/dev/ttyAMA0", 9600)
    self.ser = serial.Serial(port, baud)
    if self.ser.isOpen() != True:
      self.last_operate_status = self.STA_ERR_SERIAL

  def check_sum(self, l):
    return (l[0] + l[1] + l[2])&0x00ff

  def set_dis_range(self, min, max):
    self.distance_max = max
    self.distance_min = min

  def measure(self):
    data = []
    i = 0
    while self.ser.inWaiting() == 0:
      i += 1
      time.sleep(0.05)
      if i > 4:
        break
    i = 0
    while self.ser.inWaiting() > 0:
      data.append(ord(self.ser.read()))
      i += 1
      if data[0] != 0xff:
        i = 0
        data = []
      if i == 4:
        break
    self.ser.read(self.ser.inWaiting())
    if i == 4:
      sum = self.check_sum(data)
      if sum != data[3]:
        self.last_operate_status = self.STA_ERR_CHECKSUM
      else:
        self.distance = data[1]*256 + data[2]
        self.last_operate_status = self.STA_OK
      if self.distance > self.distance_max:
        self.last_operate_status = self.STA_ERR_CHECK_OUT_LIMIT
        self.distance = self.distance_max
      elif self.distance < self.distance_min:
        self.last_operate_status = self.STA_ERR_CHECK_LOW_LIMIT
        self.distance = self.distance_min
    else:
      self.last_operate_status = self.STA_ERR_DATA

  def get_distance(self):
    self.measure()
    return self.distance