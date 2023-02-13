/* DEVICE 2
  
   |  Arduino   |  GA6-B  |
   |------------|---------|
   |  D10 (RX)  |  U-TX   |
   |  D11 (TX)  |  U-RX   |
   
*/

#define gsm_rx 3
#define gsm_tx 2
// #define buz_pin 5
#define buz_pin LED_BUILTIN

#include <SoftwareSerial.h>

bool msg_received = 0;
String msg;

SoftwareSerial _gsm(gsm_tx, gsm_rx);


void setup() {
  Serial.begin(2400);
  _gsm.begin(2400);

  pinMode(buz_pin, OUTPUT);
  delay(500);

  _gsm.println("AT");
  delay(500);
  // updateSerial();
  _gsm.println("AT+CSQ");
  delay(500);
  // updateSerial();
  _gsm.println("AT+CCID");
  delay(500);
  // updateSerial();
  _gsm.println("AT+CREG?");
  delay(500);
  // updateSerial();

  // SEND SMS
  // _gsm.println("AT+CMGF=1");delay(500);
  // updateSerial();
  // _gsm.print("AT+CMGS=\"081228445269\"\r");delay(500);
  // updateSerial();
  // _gsm.print("Tes SMS");delay(500);
  // updateSerial();
  // _gsm.write(26); // send ASCII character of <CTRL+Z>delay(500);

  // RECEIVE SMS
  _gsm.println("AT+CMGF=1");
  delay(500);
  // updateSerial();
  _gsm.println("AT+CNMI=1,2,0,0,0");
  delay(500);
  // updateSerial();

  // MAKE A CALL
  // _gsm.println("ATD081228445269;");
  // delay(500);
  // updateSerial();
  // delay(20000);
  // _gsm.println("ATH");
  // delay(500);
  // updateSerial();
}


void loop() {
  updateSerial();
  
  if (msg.indexOf("REPORT") >= 0) {
    msg_received = 1;
  }
  
  if (msg_received) {
    for (int i = 0; i < 5; i++) {
      buzzer();
    }
    msg_received = 0;
    msg = "";
  }
}


void buzzer() {
  digitalWrite(buz_pin, HIGH);
  delay(1000);
  digitalWrite(buz_pin, LOW);
  delay(1000);
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    _gsm.write(Serial.read());
  }
  while (_gsm.available()) {
    // Serial.write(Serial.read());
    msg = _gsm.readString();
    Serial.print(msg);
  }
}
