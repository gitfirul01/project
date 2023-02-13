#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <avr/sleep.h>

#include "MAX30102.h"
#include "Pulse.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SCREEN_ADDRESS 0x3C
#define SPHYGMO_ADDRESS 0x50

#define send_btn 2
#define act_btn 3
#define BEAT_LED LED_BUILTIN
#define OPTIONS 7

#define device1_number "+6281328431180"
#define device2_number "+6281328431160"
#define doctor_number "+62xxxxxxxxxx"

//spo2_table is approximated as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t spo2_table[184] PROGMEM = { 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99,
                                          99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
                                          100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97,
                                          97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91,
                                          90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81,
                                          80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67,
                                          66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50,
                                          49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29,
                                          28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5,
                                          3, 2, 1 };

static const unsigned char PROGMEM logo2_bmp[] = {
  0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B,
  0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,
  0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01,
  0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
  0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01,
  0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
  0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B,
  0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00
};

static const unsigned char PROGMEM logo3_bmp[] = {
  0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
  0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
  0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
  0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
  0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
  0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
  0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
  0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00
};

typedef struct data_ {
  int sys;
  int dias;
};
typedef union packet_ {
  data_ value;
  byte byteArray[sizeof(data_)];
};
packet_ sphygmo;

int state;
int risk;
bool led_on = false;
bool data_available = false;

int beatAvg = 0;
int SPO2 = 0, SPO2f = 0;

long lastTime = 0, nowTime = 0;
long lastBeat = 0, now = 0;

bool filter_for_graph = false;
bool draw_Red = false;
uint8_t sleep_counter = 0;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
SoftwareSerial _tensimeter(10, 11);
// SoftwareSerial _gsm(8, 9);
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;
MAFilter bpm;



void setup() {
  Serial.begin(115200);
  _tensimeter.begin(9600);
  // _gsm.begin(2400);

  pinMode(BEAT_LED, OUTPUT);

  filter_for_graph = EEPROM.read(OPTIONS);
  draw_Red = EEPROM.read(OPTIONS + 1);

  /* begin SSD1306 */
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 28);
  display.println("Welcome");
  display.display();
  delay(2000);

  /* begin MAX30102 */
  if (!sensor.begin())
    state = 0;
  else state = 2;
  sensor.setup();

  attachInterrupt(digitalPinToInterrupt(act_btn), action_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(send_btn), send_isr, FALLING);
}



void loop() {
  /* Waiting data from tensimeter */
  if (_tensimeter.available()) {
    data_available = true;
    _tensimeter.readBytes(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    delay(500);
  }

  now = millis();
  __max30102__();

  nowTime = millis();
  if (nowTime - lastTime > 300) {
    __check_condition__();
    __ssd1306__();
  }
}



void action_isr() {
  if (state == 4)
    state = 2;
}

void send_isr() {
  if (state == 4) {
    // if (risk > 2) {
    //   String message = "Danger";
    // } else if (risk == 2) {
    //   String message = "Warning";
    // }
    // // send_sms(device2_number, message);
    // // send_sms(doctor_number, message);
    // risk = 0;

    state = 2;
  }
}

void go_sleep() {
  display.clearDisplay();
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  delay(10);
  sensor.off();
  delay(10);
  cbi(ADCSRA, ADEN);  // disable adc
  delay(10);
  pinMode(0, INPUT);
  pinMode(2, INPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();  // sleep until act_button press
  // cause reset
  setup();
}

void __max30102__() {
  sensor.check();

  if (!sensor.available()) return;

  uint32_t irValue = sensor.getIR();
  uint32_t redValue = sensor.getRed();
  sensor.nextSample();

  if (irValue < 50000) {
    // state = (sleep_counter <= 50 ? 1 : 3);
    // delay(100);
    // ++sleep_counter;
    // if (sleep_counter > 100) {
    //   go_sleep();
    //   sleep_counter = 0;
    // }
    state = 4;
  } else {
    state = 2;
    sleep_counter = 0;
    int16_t IR_signal, Red_signal;
    bool beatRed, beatIR;
    if (!filter_for_graph) {
      IR_signal = pulseIR.dc_filter(irValue);
      Red_signal = pulseRed.dc_filter(redValue);
      beatRed = pulseRed.isBeat(pulseRed.ma_filter(Red_signal));
      beatIR = pulseIR.isBeat(pulseIR.ma_filter(IR_signal));
    } else {
      IR_signal = pulseIR.ma_filter(pulseIR.dc_filter(irValue));
      Red_signal = pulseRed.ma_filter(pulseRed.dc_filter(redValue));
      beatRed = pulseRed.isBeat(Red_signal);
      beatIR = pulseIR.isBeat(IR_signal);
    }
    // check IR or Red for heartbeat
    if (draw_Red ? beatRed : beatIR) {
      long btpm = 60000 / (now - lastBeat);
      if (btpm > 0 && btpm < 200) beatAvg = bpm.filter((int16_t)btpm);
      lastBeat = now;
      digitalWrite(BEAT_LED, HIGH);
      //
      display.clearDisplay();
      display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);
      display.setTextSize(2);
      display.setCursor(42, 15);
      display.print(beatAvg);
      display.setCursor(90, 15);
      display.print(SPO2);

      display.setTextSize(1);
      display.setCursor(42, 3);
      display.print("bpm");
      display.setCursor(90, 3);
      display.print("%SpO2");
      display.setCursor(5, 41);
      display.print("Systole : ");
      display.print(sphygmo.value.sys);
      display.setCursor(5, 55);
      display.print("Diastole: ");
      display.print(sphygmo.value.dias);
      display.display();
      //
      led_on = true;
      // compute SpO2 ratio
      long numerator = (pulseRed.avgAC() * pulseIR.avgDC()) / 256;
      long denominator = (pulseRed.avgDC() * pulseIR.avgAC()) / 256;
      int RX100 = (denominator > 0) ? (numerator * 100) / denominator : 999;
      // using formula
      SPO2f = (10400 - RX100 * 17 + 50) / 100;
      // from table
      if ((RX100 >= 0) && (RX100 < 184))
        SPO2 = pgm_read_byte_near(&spo2_table[RX100]);
    }
  }
  if (led_on && (now - lastBeat) > 25) {
    digitalWrite(BEAT_LED, LOW);
    led_on = false;
  }
}

void __ssd1306__() {
  display.clearDisplay();
  switch (state) {
    case 0:
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Device not found!");
      display.display();
      while (1)
        ;
      break;
    case 1:
      display.setTextSize(1);
      display.setCursor(0, 28);
      display.println("Place your finger!");

      break;
    case 2:
      display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);
      display.setTextSize(2);
      display.setCursor(42, 15);
      display.print(beatAvg);
      display.setCursor(90, 15);
      display.print(SPO2);

      display.setTextSize(1);
      display.setCursor(42, 3);
      display.print("bpm");
      display.setCursor(90, 3);
      display.print("%SpO2");
      display.setCursor(5, 41);
      display.print("Systole : ");
      display.setCursor(65, 41);
      display.print(sphygmo.value.sys);
      display.setCursor(5, 55);
      display.print("Diastole: ");
      display.setCursor(65, 55);
      display.print(sphygmo.value.dias);

      break;
    case 3:
      display.setTextSize(1);
      display.setCursor(0, 28);
      display.println("OFF IN");
      display.write(10 - sleep_counter / 10 + '0');
      display.write('s');

      break;
    case 4:  // condition interface
      display.setTextSize(1);
      if (risk == 0) {
        display.setCursor(0, 28);
        display.println("Normal");
      }
      if (risk == 1) {
        display.setCursor(0, 25);
        display.println("Normal");
        display.println("ulangi pengamatan dalam 30 menit");
      }
      if (risk == 2) {
        display.setCursor(0, 22);
        display.println("Waspada");
        display.println("panggil dokter kandungan dan");
        display.println("ulangi pengamatan dalam 30 menit");
      }
      if (risk > 2) {
        display.setCursor(0, 19);
        display.println("Bahaya");
        display.println("peninjauan segera oleh dokter kandungan");
        display.println("dan observasi ulang dalam 15 menit");
        display.println("atau pemantauan terus menerus");
      }
      display.display();
      while (1) {
        delay(1000);
        if (state != 4) {
          break;
        }
      }
      break;
  }
  display.display();
  lastTime = nowTime;
}

void __check_condition__() {
  /* Condition checking
   * risk = 0: normal
   * risk = 1: warn
   * risk = 2: danger
   */
  if (data_available) {
    // if (SPO2 < 95) risk += 2;

    // if ((sphygmo.value.sys < 90) || (sphygmo.value.sys > 180)) risk += 2;
    // else if (((90 <= sphygmo.value.sys) && (sphygmo.value.sys <= 100)) || ((150 <= sphygmo.value.sys) && (sphygmo.value.sys <= 180))) risk += 1;

    // if (sphygmo.value.dias > 120) risk += 2;
    // else if ((100 <= sphygmo.value.dias) && (sphygmo.value.dias <= 120)) risk += 1;

    // if ((beatAvg < 40) || (beatAvg > 120)) risk += 2;
    // else if (((40 <= beatAvg) && (beatAvg <= 50)) || ((100 <= beatAvg) && (beatAvg <= 120))) risk += 1;

    data_available = 0;
  }
}

// void send_sms(String number, String message) {
//   _gsm.println("AT");
//   updateSerial();
//   _gsm.println("AT+CMGF=1");
//   updateSerial();
//   _gsm.print("AT+CMGS=\"" + number + "\"");
//   updateSerial();
//   _gsm.print(message + "\nSpO2 = " + String(SPO2) + "\nSystole = " + String(sphygmo.value.sys) + "\nDiastole = " + String(sphygmo.value.dias) + "\nPulse Rate = " + String(beatAvg));
//   updateSerial();
//   _gsm.write(26);
// }

// void updateSerial() {
//   delay(500);
//   while (Serial.available()) {
//     _gsm.write(Serial.read());
//   }
//   while(_gsm.available()) {
//     Serial.write(_gsm.read());
//   }
// }
