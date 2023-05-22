#include <SoftwareSerial.h>

SoftwareSerial _tensimeter(10, 11);

typedef struct data_ {
  int sys;
  int dias;
};
typedef union packet_ {
  data_ value;
  byte byteArray[sizeof(data_)];
};
packet_ sphygmo;


void setup() {
  Serial.begin(115200);
  _tensimeter.begin(115200);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  if (_tensimeter.available()) {
    _tensimeter.readBytes(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    delay(500);
    Serial.print("Systole : ");
    Serial.print(sphygmo.value.sys);
    Serial.print("Diastole: ");
    Serial.print(sphygmo.value.dias);
  }
}
