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
#define id_len 5

SoftwareSerial _arduino1_(10, 11);
SoftwareSerial SIM900A(8, 9);

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
  char id[id_len];
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
  SIM900A.begin(2400);

  delay(500);
  SIM900A.println("AT");
  updateSerial();
  SIM900A.println("AT+CSQ");
  updateSerial();
  SIM900A.println("AT+CCID");
  updateSerial();
  SIM900A.println("AT+CREG?");
  updateSerial();

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

    if (command.value.cmd = 'D') {
      send_sms(device2_number, "Bahaya");
      // send_sms(doctor_number, "Bahaya");
      // call(doctor_number);
    } else if (command.value.cmd = 'W') {
      send_sms(device2_number, "Waspada");
      // send_sms(doctor_number, "Waspada");
      // call(doctor_number);
    } else if (command.value.cmd = 'N') {
      send_sms(device2_number, "Normal dengan peringatan");
      // send_sms(doctor_number, "Normal dengan peringatan");
      // call(doctor_number);
    } else if (command.value.cmd != 'n') {
      send_sms(device2_number, "Normal");
      // send_sms(doctor_number, "Normal");
      // call(doctor_number);
    }
    http_post();  // kirim data ke website jika ada data masuk dari arduino 1
  }

  if (millis() - lastTime > 1000) {
    if (data_available) {
      sphygmo.value.dias = 0.8*sphygmo.value.dias + 8.4;

      _arduino1_.listen();
      _arduino1_.write(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    }
    data_available = 0;

    lastTime = millis();
  }
}



void send_sms(String number, String message) {
  SIM900A.listen();
  SIM900A.println("AT");
  updateSerial();
  SIM900A.println("AT+CMGF=1");
  updateSerial();
  SIM900A.print("AT+CMGS=\"" + number + "\"\r");
  updateSerial();
  SIM900A.print("REPORT\n\nStatus: " + message + "\nSpO2 = " + String(command.value.spo2) + "\nSys = " + String(sphygmo.value.sys) + "\nDias = " + String(sphygmo.value.dias) + "\nRate = " + String(sphygmo.value.bpm));
  updateSerial();
  SIM900A.write(26);
  delay(1000);
}

void call(String number) {
  SIM900A.listen();
  SIM900A.println("ATD" + number + ";");
  updateSerial();
  delay(20000);
  SIM900A.println("ATH");
  updateSerial();
}

void updateSerial() {
  SIM900A.listen();
  delay(500);
  while (Serial.available()) {
    SIM900A.write(Serial.read());
  }
  while (SIM900A.available()) {
    Serial.write(SIM900A.read());
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
      } else {
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


void http_post() {
  String sendtoserver;
  sendtoserver += "nama=";
  sendtoserver += command.value.id;
  sendtoserver += "&tanggallahir=";
  sendtoserver += "0000-00-00";  // yyyy-mm-dd
  sendtoserver += "&paritas=";
  sendtoserver += "-";
  sendtoserver += "&sistol=";
  sendtoserver += sphygmo.value.sys;
  sendtoserver += "&diastol=";
  sendtoserver += sphygmo.value.dias;
  sendtoserver += "&nadi=";
  sendtoserver += sphygmo.value.bpm;
  sendtoserver += "&saturasioksigen=";
  sendtoserver += command.value.spo2;

  SIM900A.println("AT");
  delay(500);

  SIM900A.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  // Connection type GPRS
  delay(2000);
  updateSerial();

  SIM900A.println("AT+SAPBR=3,1,\"APN\",\"internet\"");  // APN of the provider
  delay(3000);
  updateSerial();

  SIM900A.println("AT+SAPBR=1,1");  // Open GPRS context
  delay(3000);
  updateSerial();

  SIM900A.println("AT+SAPBR=2,1");  // Query the GPRS context
  delay(3000);
  updateSerial();

  SIM900A.println("AT+HTTPINIT");  // Initialize HTTP service
  delay(3000);
  updateSerial();

  SIM900A.println("AT+HTTPPARA=\"CID\",1");  // Set parameters for HTTP session
  delay(3000);
  updateSerial();

  SIM900A.println("AT+HTTPPARA=\"URL\",\"http://vitalsign.sogydevelop.com/datakirim\"");  // Server address, PAKAI "http", TIDAK SUPPORT "https"
  delay(5000);
  updateSerial();

  SIM900A.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
  delay(5000);
  updateSerial();

  SIM900A.println("AT+HTTPPARA=\"USERDATA\",\"Authorization: Bearer YWRtaW4xMjM0NToxMjM0NTY3OA==\"");  // Bearer token
  delay(5000);
  updateSerial();

  SIM900A.println("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000");  // POST data of certain size with maximum latency time of 10seconds for inputting the data
  Serial.println(sendtoserver);
  delay(5000);
  updateSerial();

  SIM900A.println(sendtoserver);  // Data to be sent
  delay(5000);
  updateSerial();

  SIM900A.println("AT+HTTPACTION=1");  // Start POST session
  delay(5000);
  updateSerial();

  SIM900A.println("AT+HTTPREAD");
  delay(3000);
  updateSerial();

  SIM900A.println("AT+HTTPTERM");  // Terminate HTTP service
  delay(3000);
  updateSerial();

  SIM900A.println("AT+SAPBR=0,1");  // Close GPRS context
  delay(3000);
  updateSerial();
  delay(2000);
}
