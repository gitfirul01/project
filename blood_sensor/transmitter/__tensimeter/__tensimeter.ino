/*
 * Merah (pin 1) -> 3.3V
 * Putih (pin 2) -> SCL
 * Kuning (pin 3) -> SDA
 * Hitam (pin 4) -> GND
 */

#include <Wire.h>
#include <SoftwareSerial.h>

#define device1_number "081328431180"
#define device2_number "081328431160"
#define doctor_number "081228445269"

SoftwareSerial _arduino1_(10, 11);
SoftwareSerial _gsm(8, 9);

// data to send
struct data_1 {
  int sys;
  int dias;
  int bpm;
};
union packet_1 {
  data_1 value;
  byte byteArray[sizeof(data_1)];
};
packet_1 sphygmo;

// data to receive
struct data_2 {
  int spo2;
  char cmd;
};
union packet_2 {
  data_2 value;
  byte byteArray[sizeof(data_2)];
};
packet_2 command;


bool bPrint = 0;
int count, countT;
char buff[30];

bool data_available;
long lastTime;



void setup() {
  Serial.begin(9600);
  _arduino1_.begin(9600);
  _gsm.begin(2400);

  delay(500);
  _gsm.println("AT");

  Wire.begin(0x50);
  Wire.onReceive(receiveEvent);
}



void loop() {
  _arduino1_.listen();
  while (_arduino1_.available()) {
    _arduino1_.readBytes(command.byteArray, sizeof(command.byteArray));

    Serial.print(command.value.cmd);
    Serial.print("\t");
    Serial.print(command.value.spo2);
    Serial.print("\t");
    Serial.println("done");

    if (command.value.cmd != 'N') {
      if (command.value.cmd = 'D') {
        // send_sms(device2_number, "Bahaya");
        // send_sms(doctor_number, "Bahaya");
        call(doctor_number);
      } else if (command.value.cmd = 'W') {
        // send_sms(device2_number, "Waspada");
        // send_sms(doctor_number, "Waspada");
        call(doctor_number);
      }
    }
  }

  if (millis() - lastTime > 1000) {
    if (data_available) {
      _arduino1_.listen();
      _arduino1_.write(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    }
    data_available = 0;

    lastTime = millis();
  }
}



void send_sms(String number, String message) {
  _gsm.listen();
  _gsm.println("AT");
  updateSerial();
  _gsm.println("AT+CMGF=1");
  updateSerial();
  _gsm.print("AT+CMGS=\"" + number + "\"\r");
  updateSerial();
  _gsm.print("Status: " + message + "\n\nSpO2 = " + String(command.value.spo2) + "\nSys = " + String(sphygmo.value.sys) + "\nDias = " + String(sphygmo.value.dias) + "\nRate = " + String(sphygmo.value.bpm));
  updateSerial();
  _gsm.write(26);
}

void call(String number) {
  _gsm.println("ATD" + number + ";");
  updateSerial();
  delay(20000);
  _gsm.println("ATH");
  updateSerial();
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    _gsm.write(Serial.read());
  }
  while (_gsm.available()) {
    Serial.write(_gsm.read());
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
      else {
        sphygmo.value.bpm = c;
      }
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
