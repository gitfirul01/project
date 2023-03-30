#include <Adafruit_MAX31865.h>
#include <SPI.h>

#define RREF      430.0
#define RNOMINAL  100.0

unsigned long prevTime_P = 0;
unsigned long prevTime_T = 0;

// use hardware SPI (MOSI 11, MISO 12, SCK 13) and assign CS bellow
Adafruit_MAX31865 rtd[5] = {Adafruit_MAX31865(2), 
                            Adafruit_MAX31865(3), 
                            Adafruit_MAX31865(4), 
                            Adafruit_MAX31865(5), 
                            Adafruit_MAX31865(6)};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) {
    rtd[i].begin(MAX31865_3WIRE);
  }
}


void loop() {
  if (millis() > (prevTime_P + 1000)) {
    prevTime_P = millis();

    float pressure[6];
    pressure[0] = ptRead(14);
    pressure[1] = ptRead(15);
    pressure[2] = ptRead(16);
    pressure[3] = ptRead(17);
    pressure[4] = ptRead(18);
    pressure[5] = ptRead(19);

    for (int i = 0; i < 6; i++) {
      Serial.print("Pressure: "); Serial.print(pressure[i]); Serial.println("  kPa");
    }
  }

  if (millis() > (prevTime_T + 1000)) {
    prevTime_T = millis();

    float temperature[5];
    for (int i = 0; i < 5; i++) {
      temperature [i] = rtd[i].temperature(RNOMINAL, RREF);
      Serial.print("Temperature = "); Serial.print(temperature[i]); Serial.println(" C");
    }
  }
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
