/*
    S L A V E (Receiver) > MEGA
*/

#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Nextion.h>


#define address             0     // alamat EEPROM

#define relDisp             3     // pin kontrol Relay Display (Nextion)
#define relCMPC             4     // pin kontrol Relay Kompresor
#define relFAN              5     // pin kontrol Relay FAN
#define doorSwitch          6     // pintu TUTUP = open circuit  >> PULLUP >> HIGH
                                  // pintu BUKA = close circuit >> PULLUP >> LOW
#define btn_ON              7
#define btn_OFF             8
#define btn_thresTemp_inc   9
#define btn_thresTemp_dec   10

#define DS18B20             11    // semua kabel data DS18B20 dijadikan satu, pull-up 4k7 Ohm
#define buzzer              12


OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);


bool alarm = true;                // alarm pintu
bool alarmTEMP = false;           // alarm suhu
bool reachTEMP = false;

int actBtn;
int incomingVAL;

int deviceCount = 0;
float avgTemp;
int maxTEMP;
int minTEMP;
int tmpTEMP;

const int btnPIN[20] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41};    // pin dari masing-masing button; P22 = btn1, P23 = btn2, dst
int btnState[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                            // logika dari masing-masing button; 0 = lepas, 1 = tekan

// millis
unsigned long initRECEIVE = 0;
const long intervalRECEIVE = 50;

unsigned long initTMP = 0;
const long intervalTMP = 3000;

unsigned long initSLOT = 0;
const long intervalSLOT = 1000;


// Nextion Button
NexButton b1 = NexButton(1, 1, "b1");  // Button +
NexButton b2 = NexButton(1, 2, "b2");  // Button -

// Nextion Number
NexNumber n0 = NexNumber(0, 5, "n0");  // Slot
NexNumber n1 = NexNumber(1, 4, "n1");  // Min
NexNumber n2 = NexNumber(1, 5, "n2");  // Max

// Nextion Text
NexText t1 = NexText(0, 2, "t1");      // Suhu Aktual
NexTouch *nex_listen_list[] = {
  &b1,
  &b2,
  &n0,
  &n1,
  &n2,
  &t1,
  NULL
};



void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  sensors.begin();
  pinMode(buzzer, OUTPUT);

  pinMode(relDisp, OUTPUT);
  pinMode(relCMPC, OUTPUT);
  pinMode(relFAN, OUTPUT);
  pinMode(doorSwitch, INPUT_PULLUP);
  
  pinMode(btn_ON, INPUT_PULLUP);
  pinMode(btn_OFF, INPUT_PULLUP);
  pinMode(btn_thresTemp_inc, INPUT_PULLUP);
  pinMode(btn_thresTemp_dec, INPUT_PULLUP);

  for (int btn = 0; btn < 20; btn++) {
    pinMode(btnPIN[btn], INPUT_PULLUP);
  }

  b1.attachPush(b1PushCallback);
  b2.attachPush(b2PushCallback);
}



void loop() {
  if (EEPROM.read(address) > 0) {
    maxTEMP = EEPROM.read(address);    
  } else {
    maxTEMP = 5;
  }
  minTEMP = maxTEMP - 1;
  tmpTEMP = maxTEMP;

  unsigned long nowTime = millis();
  if ( (nowTime - initRECEIVE) >= intervalRECEIVE ) {
    inputCheck();
    Nextion();
    initRECEIVE = nowTime;
  }
  if ( (nowTime - initTMP) >= intervalTMP ) {
    readTMP();
    checkTMP();
    initTMP = nowTime;
  }
  if ( (nowTime - initSLOT) >= intervalSLOT ) {
    doorCheck();
    slotCheck();
    initSLOT = nowTime;
  }

  nexLoop(nex_listen_list);
  EEPROM.update(address, maxTEMP);
}



void inputCheck() {
  incomingVAL = Serial1.read();

  if ( (incomingVAL == '1') || ((digitalRead(btn_ON) == LOW) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == HIGH)) ) {
    beeb(100);
    // Display ON
    digitalWrite(relDisp, LOW);
  }
  else if ( (incomingVAL == '2') || ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == LOW) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == HIGH)) ) {
    beeb(100);
    // Display OFF
    digitalWrite(relDisp, HIGH);
  }
  else if ( (incomingVAL == '3') || ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == LOW) && (digitalRead(btn_thresTemp_dec) == HIGH)) ) {
    beeb(100);
    reachTEMP = false;
    // thres temp increment
    maxTEMP++;
    minTEMP++;
  }
  else if ( (incomingVAL == '4') || ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == LOW)) ) {
    beeb(100);
    reachTEMP = false;
    // thres temp decrement
    minTEMP--;
    maxTEMP--;
  }
}

void readTMP() {
  avgTemp = 0;
  deviceCount = sensors.getDeviceCount();
  sensors.requestTemperatures();

  for (int i = 0;  i < deviceCount;  i++) {
    float temp = sensors.getTempCByIndex(i);
    avgTemp += temp;
  }
  avgTemp /= deviceCount;

  // jika suhu aktual < batas
  if (avgTemp < (minTEMP - 1)) {
    beeb(500);
    // matikan kompresor
    digitalWrite(relCMPC, HIGH);
  } 
  // jika suhu aktual > batas
  else if (avgTemp > (maxTEMP + 1)) {
    beeb(500);
    // nyalakan kompresor
    digitalWrite(relCMPC, LOW);
  }
}

void checkTMP() {
  if (reachTEMP == false) {
    alarmTEMP = false;
    if (avgTemp == maxTEMP || avgTemp == minTEMP || ( (avgTemp > minTEMP) && (avgTemp < maxTEMP) )) {
      reachTEMP = true;
      alarmTEMP = true;
    }
  }
  // else {
  //   alarmTEMP = true;
  //   if (maxTEMP != tmpTEMP) {
  //     reachTEMP = false;
  //   }
  // }
}

void slotCheck() {
  actBtn = 0;   // active button

  for (int btn = 0; btn < 20; btn++) {
    if ( digitalRead(btnPIN[btn]) == LOW ) {     // if button is pressed
      btnState[btn] = 1;
    } else {                                     // if button is not pressed
      btnState[btn] = 0;
    }
    actBtn += btnState[btn];          // lakukan increment tiap ada button yang aktif
  }
}

void doorCheck() {
  if ( digitalRead(doorSwitch) == HIGH ) {    // pintu menutup = HIGH
    digitalWrite(relFAN, LOW);
    alarm = true;
  } else {                                    // pintu buka = LOW
    digitalWrite(relFAN, HIGH);
    alarm = false;
  }
}

void beeb(int duration) {
  if (duration < 200) {
    digitalWrite(buzzer, HIGH);
    delay(duration);
    digitalWrite(buzzer, LOW);
  } 
  else if (alarm && alarmTEMP) {
    digitalWrite(buzzer, HIGH);
    delay(duration);
    digitalWrite(buzzer, LOW);
  }
}

// NEXTION
void b1PushCallback(void *ptr)
{
  reachTEMP = false;
  maxTEMP++;
  minTEMP++;
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void b2PushCallback(void *ptr)
{
  reachTEMP = false;
  maxTEMP--;
  minTEMP--;
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}

void Nextion()
{
  b1PushCallback(btn_thresTemp_inc);
  b2PushCallback(btn_thresTemp_dec);
  
  /*Menampilkan Suhu Rata-rata di Nextion*/
  String tempString = "t1.txt=\"" + String(avgTemp) + "\"";
  Serial2.print(tempString);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  /*Menampilkan Slot di Nextion*/
  Serial2.print("n0.val=");
  Serial2.print(actBtn);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("n2.val=");
  Serial2.print(maxTEMP);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);

  Serial2.print("n1.val=");
  Serial2.print(minTEMP);
  Serial2.write(0xff);
  Serial2.write(0xff);
  Serial2.write(0xff);
}
