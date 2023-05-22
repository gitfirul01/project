/*
 * Merah (pin 1) -> 3.3V
 * Hijau (pin 2) -> SCL
 * Kuning (pin 3) -> SDA
 * Hitam (pin 4) -> GND
 */

#include <Wire.h>

int sys, dias, bpm;

void setup() {
  Wire.begin(0x50);              // join i2c bus with address #4
  Wire.onReceive(receiveEvent);  // register event
  Serial.begin(115200);            // start serial for output
  Serial.println("done");
}

void loop() {
}

bool bPrint = 0;
int count, countT;
char buff[30];

void receiveEvent(int howMany) {
  while (0 < Wire.available())  // loop through all but the last
  {
    char c = Wire.read();  // receive byte as a character
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
        sys = c;
      } else if (count == 1) {
        dias = c;
      } else {
        bpm = c;
      }
      count++;
      if (count == 3) {
        sprintf(buff, "sys:%d, dias:%d, bpm:%d", sys, dias, bpm);
        Serial.println(buff);
        countT = 0;
        count = 0;
      }
    }
  }
}