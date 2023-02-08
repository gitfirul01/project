/*
   |  Arduino  |  GA6-B  |
   |-----------|---------|
   |  D2 (RX)  |  U-TX   |
   |  D3 (TX)  |  U-RX   |
*/

#define gsm_rx 3
#define gsm_tx 2
#define buz_pin 5

#include<SoftwareSerial.h>

bool msg_received = 0;

SoftwareSerial gsm(gsm_tx, gsm_rx);


void setup() {
  Serial.begin(115200);
  gsm.begin(115200);

  pinMode(buz_pin, OUTPUT);

  gsm.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  gsm.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  gsm.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  gsm.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
}


void loop() {
  // device 2 standby sampai menerima pesan dari device 1
  updateSerial();

  // ketika menerima pesan, maka buzzer akan berbunyi selama 10 detik
  if (msg_received) {
    for (int i = 0; i < 5; i++) {
      buzzer();
    }
  }
}


void updateSerial() {
  delay(500);
  while (Serial.available()) {
    gsm.write(Serial.read()); //Forward what serial port received to GSM
  }
  while (gsm.available()) {
    msg_received = 1;
    Serial.write(gsm.read()); //Forward what GSM received to serial port
  }
}

void buzzer() {
  digitalWrite(buz_pin, HIGH);
  delay(1000);
  digitalWrite(buz_pin, LOW);
  delay(1000);
}
