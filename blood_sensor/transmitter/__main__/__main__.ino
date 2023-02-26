#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
#define max_measurement 1

#define BEAT_LED LED_BUILTIN
#define select_btn_pin 2
#define change_btn_pin 3
#define tensimeter_pin 9

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

// data to receive
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

// data to send
struct data_2 {
  int spo2;
  char cmd;
};
union packet_2 {
  data_2 value;
  byte byteArray[sizeof(data_2)];
};
packet_2 command;


int risk, last_risk;
int beatAvg = 0;
int SPO2 = 0, SPO2f = 0;

int page;
int menu = 1;

bool led_on = false;         // true jika led sedang menyala, false jika led mati
bool tensimeter_on = false;  // true jika tensimeter sedang menyala, false jika tensimeter mati
bool repeat_flag = false;    // true untuk melakukan pengulangan, false setelah melakukan pengulangan (sudah selesai)
bool on_repeat = false;      // true jika sudah pernah melakukan pengulangan, false jika masih dalam pengukuran pertama
int repeat_countdown = 300;  // countdown 5 menit untuk repeat

long now = 0;
long lastTime = 0, lastBeat = 0;
uint8_t sleep_counter = 0;

int measurement_counter = 0;       // sudah berapa kali pengukuran
int measurement_countdown = 60;    // countdown sebelum pengukuran berikutnya
long start_measurement_count = 0;  // variabel waktu millis
long last_measurement_count = 0;   // variabel waktu millis



Adafruit_SSD1306 display(128, 64, &Wire, -1);
SoftwareSerial _arduino2_(10, 11);
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;
MAFilter bpm;



void setup() {
  Serial.begin(9600);
  _arduino2_.begin(9600);

  pinMode(change_btn_pin, INPUT);
  pinMode(select_btn_pin, INPUT);
  pinMode(BEAT_LED, OUTPUT);
  pinMode(tensimeter_pin, OUTPUT);

  digitalWrite(tensimeter_pin, HIGH);  // tensimeter off -> low trigger

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
  display.println(F("Welcome"));
  display.display();
  delay(2000);

  /* begin MAX30102 */
  if (!sensor.begin())
    page = 0;
  else page = 2;
  sensor.setup();

  attachInterrupt(digitalPinToInterrupt(change_btn_pin), change_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(select_btn_pin), select_isr, FALLING);
}



void loop() {
  now = millis();
  /* Read SPO2 and BPM continuously */
  __max30102__();

  /* Waiting data from tensimeter */
  if (_arduino2_.available()) {
    _arduino2_.readBytes(sphygmo.byteArray, sizeof(sphygmo.byteArray));
    tensimeter_on = false;
    measurement_counter++;

    digitalWrite(tensimeter_pin, LOW);  // reset tensimeter setelah penggunaan
    delay(300);
    digitalWrite(tensimeter_pin, HIGH);
  }

  /* Measurement timer and counter */
  if (((measurement_counter > 0 || repeat_flag) && measurement_counter <= max_measurement) && !tensimeter_on) {
    start_measurement_count = millis();                              // update millis
    if (start_measurement_count - last_measurement_count >= 1000) {  // blok untuk mengurangi countdown
      if (repeat_flag) {                                             // kalau repeat flag aktif
        repeat_countdown--;                                          // kurangi countdown repeat 300 detik
      } else {                                                       // kalau repeat flag tdk aktif
        measurement_countdown--;                                     // kurangi countdown measurement 60 detik
      }
      last_measurement_count = start_measurement_count;  // update last time
    }
    if (measurement_countdown == 0 || repeat_countdown == 0) {  // jika countdown measurement habis, nyalakan tensimeter
      digitalWrite(tensimeter_pin, LOW);                        // trigger tensimeter
      delay(300);
      digitalWrite(tensimeter_pin, HIGH);

      if (repeat_flag) {      // jika countdown repeat habis,
        repeat_flag = false;  // pengulangan selesai
        on_repeat = true;     // sudah melakukan pengulangan
      }
      measurement_countdown = 60;  // reset variabel
      repeat_countdown = 300;
      tensimeter_on = true;
    }
  }

  /* Display */
  if (now - lastTime > 300) {
    __check_condition__();
    __ssd1306__();

    // Serial.print("tensimeter_on"); Serial.print("\t:"); Serial.println(tensimeter_on);
    Serial.print("repeat_flag");
    Serial.print("\t:");
    Serial.println(repeat_flag);
    Serial.print("on_repeat");
    Serial.print("\t:");
    Serial.println(on_repeat);
    Serial.print("repeat_countdown");
    Serial.print("\t:");
    Serial.println(repeat_countdown);
    // Serial.print("measurement_counter"); Serial.print("\t:"); Serial.println(measurement_counter);
    // Serial.print("measurement_countdown"); Serial.print("\t:"); Serial.println(measurement_countdown);
  }
}



void change_isr() {
  if (page == 4) {
    menu++;
    if (menu > 2) menu = 1;
  }
}

void select_isr() {
  if (page == 4) {
    if (menu == 1) {  // jika menu send terpilih
      command.value.spo2 = SPO2;
      _arduino2_.write(command.byteArray, sizeof(command.byteArray));

      // Serial.print(command.value.cmd); Serial.print("\t");
      // Serial.print(command.value.bpm); Serial.print("\t");
      // Serial.print(command.value.spo2); Serial.print("\t");
      // Serial.println("done");
    }
    risk = 0;
    page = 2;
    menu = 1;
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

  if (irValue < 5000) {
    page = (page == 4 ? 4 : (sleep_counter <= 100 ? 1 : 3));
    delay(100);
    ++sleep_counter;
    if (sleep_counter > 150) {
      go_sleep();
      sleep_counter = 0;
    }
  } else {
    page = (page == 4 ? 4 : 2);
    sleep_counter = 0;
    int16_t IR_signal, Red_signal;
    bool beatRed, beatIR;
    // if (!filter_for_graph) {
    IR_signal = pulseIR.dc_filter(irValue);
    Red_signal = pulseRed.dc_filter(redValue);
    beatRed = pulseRed.isBeat(pulseRed.ma_filter(Red_signal));
    beatIR = pulseIR.isBeat(pulseIR.ma_filter(IR_signal));
    // check IR or Red for heartbeat
    if (beatIR) {
      long btpm = 60000 / (now - lastBeat);
      if (btpm > 0 && btpm < 200) beatAvg = bpm.filter((int16_t)btpm);
      lastBeat = now;
      digitalWrite(BEAT_LED, HIGH);
      //
      if (page == 2) {
        display.clearDisplay();
        display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);
        display.setTextSize(2);
        display.setCursor(42, 15);
        display.print(beatAvg);
        display.setCursor(90, 15);
        display.print(SPO2);

        display.setTextSize(1);
        display.setCursor(42, 3);
        display.print(F("bpm"));
        display.setCursor(90, 3);
        display.print(F("%SpO2"));
        display.setCursor(5, 41);
        display.print(F("Systole : "));
        display.print(sphygmo.value.sys);
        display.setCursor(5, 55);
        display.print(F("Diastole: "));
        display.print(sphygmo.value.dias);

        display.setCursor(115, 41);
        display.print(measurement_counter);
        display.setCursor(98, 55);
        if (repeat_flag) {
          display.print(repeat_countdown / 60);
          display.print(":");
          if (repeat_countdown % 60 < 10) {
            display.print(0);
          }
          display.print(repeat_countdown % 60);
        } else {
          if (measurement_countdown < 10) display.setCursor(115, 55);
          else display.setCursor(108, 55);
          display.print(measurement_countdown);
        }

        display.display();
      }
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
  switch (page) {
    case 0:
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(F("Device not found!"));
      display.display();
      while (1)
        ;
      break;
    case 1:
      display.setTextSize(1);
      display.setCursor(0, 28);
      display.println(F("Place your finger!"));

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
      display.print(F("bpm"));
      display.setCursor(90, 3);
      display.print(F("%SpO2"));
      display.setCursor(5, 41);
      display.print(F("Systole : "));
      display.setCursor(65, 41);
      display.print(sphygmo.value.sys);
      display.setCursor(5, 55);
      display.print(F("Diastole: "));
      display.setCursor(65, 55);
      display.print(sphygmo.value.dias);

      display.setCursor(115, 41);
      display.print(measurement_counter);
      display.setCursor(98, 55);
      if (repeat_flag) {
        display.print(repeat_countdown / 60);
        display.print(":");
        if (repeat_countdown % 60 < 10) {
          display.print(0);
        }
        display.print(repeat_countdown % 60);
      } else {
        if (measurement_countdown < 10) display.setCursor(115, 55);
        else display.setCursor(108, 55);
        display.print(measurement_countdown);
      }

      break;
    case 3:
      display.setTextSize(1);
      display.setCursor(0, 28);
      display.println(F("OFF IN"));
      display.write(10 - (sleep_counter - 50) / 10 + '0');
      display.write(' s');

      break;
    case 4:  // condition interface
      display.setTextSize(1);
      display.setCursor(0, 0);

      if (risk == 0) {
        display.println(F("Normal"));
        command.value.cmd = 'n';  // normal
        repeat_flag = false;

      } else if (risk == 1) {
        display.println(F("Normal"));
        command.value.cmd = 'N';  // normal, dengan peringatan
        if (!on_repeat) {         // jika tidak dalam keadaan repeat (belum pernah melakukan repeat),
          repeat_flag = true;     // maka lakukan repeat
        }

      } else if (risk == 2) {
        display.println(F("Waspada"));
        command.value.cmd = 'W';  // warning
        if (!on_repeat) {         // jika tidak dalam keadaan repeat (belum pernah melakukan repeat),
          repeat_flag = true;     // maka lakukan repeat
        }

      } else if (risk > 2) {
        display.println(F("Bahaya"));
        command.value.cmd = 'D';  // danger
        if (!on_repeat) {         // jika tidak dalam keadaan repeat (belum pernah melakukan repeat),
          repeat_flag = true;     // maka lakukan repeat
        }
      }

      display.setCursor(0, 32);
      if (menu == 1) display.println(F("> Send"));
      else display.println(F("  Send"));

      if (menu == 2) display.println(F("> Back"));
      else display.println(F("  Back"));

      break;
  }
  display.display();
  lastTime = now;
}

void __check_condition__() {
  /* Condition checking
   * risk = 0: normal
   * risk = 1: warn
   * risk = 2: danger
   */
  if (measurement_counter == max_measurement) {  // cek kondisi jika jumlah pengukuran sudah mencapai batas maksimal
    if (SPO2 < 95) risk += 2;

    if ((sphygmo.value.sys < 90) || (sphygmo.value.sys > 180)) risk += 2;
    else if (((90 <= sphygmo.value.sys) && (sphygmo.value.sys <= 100)) || ((150 <= sphygmo.value.sys) && (sphygmo.value.sys <= 180))) risk += 1;

    if (sphygmo.value.dias > 120) risk += 2;
    else if ((100 <= sphygmo.value.dias) && (sphygmo.value.dias <= 120)) risk += 1;

    if ((sphygmo.value.bpm < 40) || (sphygmo.value.bpm > 120)) risk += 2;
    else if (((40 <= sphygmo.value.bpm) && (sphygmo.value.bpm <= 50)) || ((100 <= sphygmo.value.bpm) && (sphygmo.value.bpm <= 120))) risk += 1;

    last_risk = risk;
    page = 4;
    measurement_counter = 0;
  }
}
