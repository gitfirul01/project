/*
 * Merah (pin 1) -> 3.3V
 * Putih (pin 2) -> SCL
 * Kuning (pin 3) -> SDA
 * Hitam (pin 4) -> GND
 */

#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial _port1(10, 11);

typedef struct data_ {
  int sys;
  int dias;
};
typedef union packet_ {
  data_ value;
  byte byteArray[sizeof(data_)];
};
packet_ sphygmo;

bool bPrint = 0;
int count, countT;
char buff[30];

bool data_available;
long lastTime;

void setup() {
  Serial.begin(9600);
  _port1.begin(9600);
  delay(500);

  Wire.begin(0x50);
  Wire.onReceive(receiveEvent);

  // Serial.println("done");
//   sphygmo.value.sys = 0;
// sphygmo.value.dias = 0;
}

void loop() {
  if (millis() - lastTime > 1000) {
    if (data_available) {
      _port1.write(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    }
    data_available = 0;

// sphygmo.value.sys++;
// sphygmo.value.dias++;

    lastTime = millis();
  }
}

void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    char c = Wire.read();
    if (countT < 4) {
      if (c == 'A') {
        countT = 1;
      }
      if (c == '9') {
        countT++;
      }
      if (c == '1') {
        countT++;
      }
      if (c == '0') {
        countT++;
      }
    } else if (countT == 4) {
      Serial.write(c);
      if (count == 0) {
        sphygmo.value.sys = c;
      } else if (count == 1) {
        sphygmo.value.dias = c;
      }
      // else {
      //   sphygmo.value.bpm = c;
      // }
      count++;
      if (count == 3) {
        // sprintf(buff, "sys:%d, dias:%d, bpm:%d", sphygmo.value.sys, sphygmo.value.dias, sphygmo.value.bpm);
        sprintf(buff, "sys:%d, dias:%d", sphygmo.value.sys, sphygmo.value.dias);
        Serial.println(buff);
        countT = 0;
        count = 0;

        data_available = 1;
      }
    }
  }
}
