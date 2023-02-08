#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_ADDRESS 0x3C
#define SPHYGMO_ADDRESS 0x50
// #define MAX30102_ADDRESS 0x

#define send_btn 2
#define act_btn 3

Adafruit_SSD1306 display(128, 64, &Wire, -1);


unsigned long lastTime, nowTime;
int state;

void setup() {
  Serial.begin(115200);

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

  attachInterrupt(digitalPinToInterrupt(send_btn), send, FALLING);
  attachInterrupt(digitalPinToInterrupt(act_btn), action, FALLING);
}

void loop() {
  nowTime = millis();
  if (nowTime - lastTime > 300) {
    display.clearDisplay();

    switch (state) {
      case 0:
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 28);
        display.println("Hello 0");

        break;
      case 1:
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 28);
        display.println("Hello 1");

        break;
    }


    display.display();
    lastTime = nowTime;
  }
}


void send() {
}


void action() {
  state++;
  if (state > 1) state = 0;
}
