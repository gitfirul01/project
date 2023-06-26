/*
*  MAX30102 driver - uses a combination of ideas from the Maxim & Sparkfun drivers
*                    used Technolbogy's TinyI2C 
*
* j.n.magee 15-10-2019
*/
#define SDA_PORT PORTC
#define SDA_PIN 0       // pin A0
#define SCL_PORT PORTC
#define SCL_PIN 1       // pin A1

#include <SoftWire.h>
SoftWire wire1;

// #include "Wire.h"
#include "MAX30102.h"


static const uint8_t MAX_30102_ID = 0x15;

MAX30102::MAX30102() {
  // Constructor
}

boolean MAX30102::begin(uint8_t i2caddr) {
  wire1.begin();
  _i2caddr = i2caddr;
  if (readRegister8(REG_PART_ID) != MAX_30102_ID)  return false; 
  return true;
}

void MAX30102::setup() {
   writeRegister8(REG_MODE_CONFIG,0x40); //reset
   delay(500);
   writeRegister8(REG_FIFO_WR_PTR,0x00);//FIFO_WR_PTR[4:0]
   writeRegister8(REG_OVF_COUNTER,0x00);//OVF_COUNTER[4:0]
   writeRegister8(REG_FIFO_RD_PTR,0x00); //FIFO_RD_PTR[4:0]
   writeRegister8(REG_FIFO_CONFIG,0x4f); //sample avg = 4, fifo rollover=false, fifo almost full = 17
   writeRegister8(REG_MODE_CONFIG,0x03); //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
   writeRegister8(REG_SPO2_CONFIG,0x27); // SPO2_ADC=4096nA, SPO2 sample rate(100Hz), pulseWidth (411uS)
   writeRegister8(REG_LED1_PA,0x17); //Choose value for ~ 6mA for LED1 (IR)
   writeRegister8(REG_LED2_PA,0x17); // Choose value for ~ 6mA for LED2 (Red)
   writeRegister8(REG_PILOT_PA,0x1F); // Choose value for ~ 6mA for Pilot LED
}

//Tell caller how many samples are available
uint8_t MAX30102::available(void) {
  int8_t numberOfSamples = sense.head - sense.tail;
  if (numberOfSamples < 0) numberOfSamples += STORAGE_SIZE;
  return (numberOfSamples);
}

//Report the next Red value in the FIFO
uint32_t MAX30102::getRed(void) {
  return (sense.red[sense.tail]);
}

//Report the next IR value in the FIFO
uint32_t MAX30102::getIR(void) {
  return (sense.IR[sense.tail]);
}

//Advance the tail
void MAX30102::nextSample(void) {
  if(available()) {
    sense.tail++;
    sense.tail %= STORAGE_SIZE; //Wrap condition
  }
}

// check sensor for new samples and upload if available
uint16_t MAX30102::check(void) {
  byte readPointer = readRegister8(REG_FIFO_RD_PTR);
  byte writePointer = readRegister8(REG_FIFO_WR_PTR);
  int numberOfSamples = 0;
  if (readPointer != writePointer) {
    //Calculate the number of readings we need to get from sensor
    numberOfSamples = writePointer - readPointer;
    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition
    int bytesLeftToRead = numberOfSamples * 6; //3 bytes each for Red and IR    
    wire1.beginTransmission(_i2caddr);
    wire1.write(REG_FIFO_DATA);
    wire1.endTransmission();
    bytesLeftToRead = bytesLeftToRead<=32? bytesLeftToRead : 32;
    wire1.requestFrom((uint8_t)_i2caddr, (uint8_t)bytesLeftToRead);      
    while (bytesLeftToRead > 0) {
        sense.head++; //Advance the head of the storage struct
        sense.head %= STORAGE_SIZE; //Wrap condition
        sense.IR[sense.head] = readFIFOSample(); 
        //Burst read three more bytes - IR  
		    sense.red[sense.head] = readFIFOSample();
        bytesLeftToRead -= 6;
    }
    wire1.endTransmission();
  } 
  return (numberOfSamples);
}
  
//
// Low-level I2C Communication
//
uint8_t MAX30102::readRegister8(uint8_t reg) {
    uint8_t value;
    wire1.beginTransmission(_i2caddr);
    wire1.write((uint8_t)reg);
    wire1.endTransmission();
    wire1.requestFrom(_i2caddr, (byte)1);
    value = wire1.read();
    wire1.endTransmission();
    return value;
}

uint32_t MAX30102::readFIFOSample() {
    byte temp[4]; 
    uint32_t temp32;
    temp[3] = 0;
    temp[2] = wire1.read();
    temp[1] = wire1.read();
    temp[0] = wire1.read();
    memcpy(&temp32, temp, 4);	
    return temp32 & 0x3FFFF;	
}

void MAX30102::writeRegister8(uint8_t reg, uint8_t value) {
  wire1.beginTransmission(_i2caddr);
  wire1.write(reg);
  wire1.write(value);
  wire1.endTransmission();
}