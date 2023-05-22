// #include <Wire.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <avr/sleep.h>

#include "Adafruit_SSD1306.h"
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

Adafruit_SSD1306 display(128, 64);
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;
MAFilter bpm;

int beatAvg = 0;
int SPO2 = 0, SPO2f = 0;

long lastTime = 0, nowTime = 0;
long lastBeat = 0;

bool led_on = false;
int state;

int voltage;
bool filter_for_graph = false;
bool draw_Red = false;
uint8_t pcflag = 0;
uint8_t istate = 0;
uint8_t sleep_counter = 0;

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


void setup() {
  Serial.begin(115200);
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
  if (!sensor.begin()) {
    state = 0;
    while (1)
      ;
  } else state = 3;
  sensor.setup();

  attachInterrupt(digitalPinToInterrupt(act_btn), action_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(send_btn), send_isr, FALLING);
}


void loop() {
  /* MAX30102 routine */
  sensor.check();
  long now = millis();

  if (!sensor.available()) return;

  uint32_t irValue = sensor.getIR();
  uint32_t redValue = sensor.getRed();
  sensor.nextSample();

  if (irValue < 5000) {
    state = (sleep_counter <= 50 ? 1 : 4);  // finger not down message
    delay(100);
    ++sleep_counter;
    if (sleep_counter > 100) {
      go_sleep();
      sleep_counter = 0;
    }
  } else {
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
  // flash led on beat for 25 ms
  if (led_on && (now - lastBeat) > 25) {
    digitalWrite(BEAT_LED, LOW);
    led_on = false;
  }

  /* SSD1306 routine */
  nowTime = millis();
  if (nowTime - lastTime > 300) {
    display.clearDisplay();
    switch (state) {
      case 0:
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Device not found!");

        break;
      case 1:
        display.setTextSize(1);
        display.setCursor(0, 28);
        display.println("Place your finger!");
        if (irValue > 5000) {
          state = 2;
        }
        break;
      case 2:
        display.setTextSize(1);
        display.setCursor(0, 28);
        display.print("Pulse Rate: ");
        display.println(beatAvg);
        display.print("SpO2: ");
        display.println(SPO2);
        break;
      case 3:
        display.setTextSize(1);
        display.setCursor(0, 28);
        display.println("Wait until receiveing the data");

        break;
      case 4:
        display.setTextSize(1);
        display.setCursor(0, 28);
        display.println("OFF IN");
        display.write(10 - sleep_counter / 10 + '0');
        display.write('s');

        break;
    }
    display.display();
    lastTime = nowTime;
  }
}



void action_isr() {
}

void send_isr() {
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
