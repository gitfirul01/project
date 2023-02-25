/* DEVICE 2
  
   |  Arduino  |  SIM900A  |
   |-----------|-----------|
   |  D3 (RX)  |   U-TX    |
   |  D2 (TX)  |   U-RX    |
   
*/

#define gsm_rx 3
#define gsm_tx 2
// #define buz_pin 5
#define buz_pin LED_BUILTIN

#include <SoftwareSerial.h>

bool msg_received = 0;
String msg;

SoftwareSerial SIM900A(gsm_tx, gsm_rx);


void setup() {
  Serial.begin(2400);
  SIM900A.begin(2400);

  pinMode(buz_pin, OUTPUT);
  delay(500);

  SIM900A.println("AT");
  updateSerial();
  SIM900A.println("AT+CSQ");
  updateSerial();
  SIM900A.println("AT+CCID");
  updateSerial();
  SIM900A.println("AT+CREG?");
  updateSerial();

  // SEND SMS
  // SIM900A.println("AT+CMGF=1");
  // updateSerial();
  // SIM900A.print("AT+CMGS=\"081228445269\"\r");
  // updateSerial();
  // SIM900A.print("Tes SMS");
  // updateSerial();
  // SIM900A.write(26); // send ASCII character of <CTRL+Z>, or 0x1A

  // RECEIVE SMS
  // SIM900A.println("AT+CMGF=1"); updateSerial();
  // SIM900A.println("AT+CNMI=1,2,0,0,0"); updateSerial();

  http_post();
  // cip_post();

  //  MAKE A CALL
  // SIM900A.println("ATD081228445269;");
  // updateSerial();
  // delay(20000);
  // SIM900A.println("ATH");
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
    SIM900A.write(Serial.read());
  }
  while (SIM900A.available()) {
    // Serial.write(Serial.read());
    msg = SIM900A.readString();
    Serial.print(msg);
  }
}


void http_post() {
  String nama = "-";
  String tanggallahir = "-";
  String paritas = "100";
  String sistol = "111";
  String diastol = "222";
  String nadi = "333";
  String saturasioksigen = "444";

  String sendtoserver;
  sendtoserver += "nama=";
  sendtoserver += nama;
  sendtoserver += "&tanggallahir=";
  sendtoserver += tanggallahir;
  sendtoserver += "&paritas=";
  sendtoserver += paritas;
  sendtoserver += "&sistol=";
  sendtoserver += sistol;
  sendtoserver += "&diastol=";
  sendtoserver += diastol;
  sendtoserver += "&nadi=";
  sendtoserver += nadi;
  sendtoserver += "&saturasioksigen=";
  sendtoserver += saturasioksigen;

  SIM900A.println("AT");
  delay(500);

  SIM900A.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  // Connection type GPRS
  delay(2000);
  updateSerial();

  SIM900A.println("AT+SAPBR=3,1,\"APN\",\"indosatgprs\"");  // APN of the provider
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

// void cip_post() {
//   String nama = "ucup";
//   String tanggallahir = "2003-12-30";
//   String paritas = "100";
//   String sistol = "111";
//   String diastol = "222";
//   String nadi = "333";
//   String saturasioksigen = "444";

//   String sendtoserver;
//   sendtoserver += "nama=";
//   sendtoserver += nama;
//   sendtoserver += "&tanggallahir=";
//   sendtoserver += tanggallahir;
//   sendtoserver += "&paritas=";
//   sendtoserver += paritas;
//   sendtoserver += "&sistol=";
//   sendtoserver += sistol;
//   sendtoserver += "&diastol=";
//   sendtoserver += diastol;
//   sendtoserver += "&nadi=";
//   sendtoserver += nadi;
//   sendtoserver += "&saturasioksigen=";
//   sendtoserver += saturasioksigen;

//   SIM900A.println("AT");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CPIN?");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CREG?");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CGATT?");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIPSHUT");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIPSTATUS");
//   delay(2000);
//   updateSerial();
//   SIM900A.println("AT+CIPMUX=0");
//   delay(2000);
//   updateSerial();
//   // SIM900A.println("AT+CGATT=1");
//   // delay(1000);
//   // updateSerial();
//   SIM900A.println("AT+CSTT=\"internet\"");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIICR");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIFSR");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIPSPRT=0");
//   delay(1000);
//   updateSerial();
//   SIM900A.println("AT+CIPSTART=\"TCP\",\"https://vitalsign.sogydevelop.com\",\"80\"");
//   delay(10000);
//   updateSerial();

//   String postRequest = "POST /datakirim HTTP/1.1\r\nAuthorization: Bearer YWRtaW4xMjM0NToxMjM0NTY3OA==\r\nHost: vitalsign.sogydevelop.com\r\nAccept: application/json\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: " + String(sendtoserver.length()) + "\r\n\r\n" + sendtoserver;
//   SIM900A.println("AT+CIPSEND=" + String(postRequest.length()));
//   delay(3000);
//   updateSerial();
//   SIM900A.print(postRequest);
//   delay(1000);
//   SIM900A.write(26);
//   delay(5000);
//   updateSerial();
// }
