/* DEVICE 2
  
   |  Arduino  |  GA6-B  |
   |-----------|---------|
   |  D2 (RX)  |  U-TX   |
   |  D3 (TX)  |  U-RX   |
   
*/

#define gsm_rx 3
#define gsm_tx 2
#define buz_pin LED_BUILTIN

#include <SoftwareSerial.h>

bool msg_received = 0;
String msg;

SoftwareSerial _gsm(gsm_tx, gsm_rx);


void setup() {
  Serial.begin(115200);
  _gsm.begin(115200);

  pinMode(buz_pin, OUTPUT);

  delay(1000);

  _gsm.println("AT");
  _gsm.println("AT+CMGF=1");
  _gsm.println("AT+CNMI=1,2,0,0,0");
}


void loop() {
  // device 2 standby sampai menerima pesan dari device 1
  delay(500);
  while (Serial.available()) {
    _gsm.write(Serial.read());
  }
  while (_gsm.available()) {
    Serial.write(_gsm.read());
    msg_received = 1;
  }
  
  // ketika menerima pesan, maka buzzer akan berbunyi selama 10 detik
  if (msg_received) {
    for (int i = 0; i < 5; i++) {
      buzzer();
    }
    msg_received = 0;
  }
}


void buzzer() {
  digitalWrite(buz_pin, HIGH);
  delay(1000);
  digitalWrite(buz_pin, LOW);
  delay(1000);
}
