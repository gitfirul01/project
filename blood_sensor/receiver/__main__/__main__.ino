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
  /*
      change baudrate to 2400 bps to solve the problem
      AT+IPR=2400   (change baudrate)
      AT&W          (save / write to memory)
  */
  Serial.begin(2400);
  _gsm.begin(2400);

  pinMode(buz_pin, OUTPUT);

  delay(1000);
  _gsm.println("AT");
  updateSerial();
  _gsm.println("AT+CSQ");
  updateSerial();
  _gsm.println("AT+CCID");
  updateSerial();
  _gsm.println("AT+CREG?");
  updateSerial();

  // SEND SMS
  _gsm.println("AT+CMGF=1");
  updateSerial();
  _gsm.print("AT+CMGS=\"081228445269\"\r");
  updateSerial();
  _gsm.print("Tes SMS");
  updateSerial();
  _gsm.write(26); // send ASCII character of <CTRL+Z>

  // RECEIVE SMS
  // _gsm.println("AT");
  // updateSerial();
  // _gsm.println("AT+CMGF=1");
  // updateSerial();
  // _gsm.println("AT+CNMI=1,2,0,0,0");
  // updateSerial();

  // MAKE A CALL
  // _gsm.println("ATD081228445269;");
  // updateSerial();
  // delay(20000);
  // _gsm.println("ATH");
  // updateSerial();
}


void loop() {
  // device 2 standby sampai menerima pesan dari device 1
  updateSerial();

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

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    _gsm.write(Serial.read());
  }
  while (_gsm.available()) {
    Serial.write(_gsm.read());
  }
}
