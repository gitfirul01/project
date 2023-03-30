#include <Adafruit_MAX31865.h>
#include <SPI.h>

#define filterWeight 0.8

#define pc1 1
#define pc2 1
#define pc3 1
#define pc4 1
#define pc5 1
#define pc6 1

#define tc1 1.0249
#define tc2 0.9894
#define tc3 0.9732
#define tc4 1.0000
#define tc5 0.9978

#define RREF      430.0
#define RNOMINAL  100.0

unsigned long prevTime_P = 0;
unsigned long prevTime_T = 0;
unsigned long prevTime_Send = 0;

float pressure[6];
float temperature[5];
float prev_pressure[6] = {0,0,0,0,0,0};
float prev_temperature[5] = {0,0,0,0,0};
bool first = true;

// use hardware SPI (MOSI 11, MISO 12, SCK 13) and assign CS bellow
Adafruit_MAX31865 rtd[5] = {Adafruit_MAX31865(PB3), 
                            Adafruit_MAX31865(PB4), 
                            Adafruit_MAX31865(PB5), 
                            Adafruit_MAX31865(PB6), 
                            Adafruit_MAX31865(PB7)};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) {
    rtd[i].begin(MAX31865_3WIRE);
  }
}


void loop() {
  if (millis() >= (prevTime_P + 1000)) {
    prevTime_P = millis();
  
    // read data
    pressure[0] = ptRead(PA0)*pc1;
    pressure[1] = ptRead(PA1)*pc2;
    pressure[2] = ptRead(PA2)*pc3;
    pressure[3] = ptRead(PA3)*pc4;
    pressure[4] = ptRead(PA4)*pc5;
    pressure[5] = ptRead(PB0)*pc6;

    for(int i = 0; i < 6; i++){
      prev_pressure[i] = pressure[i];
    }

    // filtering
    if (first != true) {
      pressure[0] = filterWeight*pressure[0] + (1-filterWeight)*prev_pressure[0];
      pressure[1] = filterWeight*pressure[1] + (1-filterWeight)*prev_pressure[1];
      pressure[2] = filterWeight*pressure[2] + (1-filterWeight)*prev_pressure[2];
      pressure[3] = filterWeight*pressure[3] + (1-filterWeight)*prev_pressure[3];
      pressure[4] = filterWeight*pressure[4] + (1-filterWeight)*prev_pressure[4];
      pressure[5] = filterWeight*pressure[5] + (1-filterWeight)*prev_pressure[5];
    }
  }

  if (millis() >= (prevTime_T + 1000)) {
    prevTime_T = millis();

    // read data
    temperature[0] = rtd[0].temperature(RNOMINAL, RREF)*tc1;
    temperature[1] = rtd[1].temperature(RNOMINAL, RREF)*tc2;
    temperature[2] = rtd[2].temperature(RNOMINAL, RREF)*tc3;
    temperature[3] = rtd[3].temperature(RNOMINAL, RREF)*tc4;
    temperature[4] = rtd[4].temperature(RNOMINAL, RREF)*tc5;

    for(int i = 0; i < 5; i++){
      prev_temperature[i] = temperature[i];
    }

    // filtering
    if (first != true) {
      temperature[0] = filterWeight*temperature[0] + (1-filterWeight)*prev_temperature[0];
      temperature[1] = filterWeight*temperature[1] + (1-filterWeight)*prev_temperature[1];
      temperature[2] = filterWeight*temperature[2] + (1-filterWeight)*prev_temperature[2];
      temperature[3] = filterWeight*temperature[3] + (1-filterWeight)*prev_temperature[3];
      temperature[4] = filterWeight*temperature[4] + (1-filterWeight)*prev_temperature[4];
    }
  }

  if (millis() >= (prevTime_Send + 1000)) {
    prevTime_Send = millis();

    for (int i = 0; i < 6; i++) {
//      Serial.print("Pressure"); Serial.print(i+1); Serial.print(": "); 
      Serial.println(pressure[i]); 
//      Serial.println("  kPa");
    }
    for (int i = 0; i < 5; i++) {
//      Serial.print("Temperature"); Serial.print(i+1); Serial.print(": "); 
      Serial.println(temperature[i]); 
//      Serial.println(" C");
    }
  }
  first = false;
}


float ptRead(uint8_t adcPin) {
  float sensorVal = analogRead(adcPin);
  float voltage = sensorVal * 5 * 1000 / 1024;
  float pres = (voltage - 500) / 400;

  return pres;
}

/*
float rtdRead() {
//  uint16_t rtd = thermo.readRTD();
//  float ratio = rtd;
//  ratio /= 32768;
  float temp = thermo.temperature(RNOMINAL, RREF);

  return temp;
}
*/
