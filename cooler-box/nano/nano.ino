/*
    M A S T E R (Transceiver) > NANO
*/

#define btn_ON              5
#define btn_OFF             4
#define btn_thresTemp_inc   11
#define btn_thresTemp_dec   12

int sendVAL;

/* == OLED == */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels

const unsigned char BLOOD[] PROGMEM = {
  B00000000, B00011000, B00000000,
  B00000000, B00011000, B00000000,
  B00000000, B00011000, B00000000,
  B00000000, B00011000, B00000000,
  B00000000, B00011000, B00000000,
  B00000000, B00011000, B00000000,
  B00000000, B00111000, B00000000,
  B00000000, B00110000, B00000000,
  B00000000, B01110000, B00000000,
  B00000000, B01100000, B00000000,
  B00000000, B01100000, B00000000,
  B00000000, B11000000, B00000000,
  B00000000, B11000000, B00000000,
  B00000000, B11000000, B00000000,
  B00000000, B11000000, B00000000,
  B00000000, B11000000, B00000000,
  B00000000, B11100000, B00000000,
  B00000010, B11100100, B00000000,
  B00000111, B01101110, B00000000,
  B00000111, B01101110, B00000000,
  B00000110, B01101110, B00000000,
  B00011111, B11111111, B10000000,
  B01111111, B11111111, B11100000,
  B01111111, B11111111, B11100000,
  B11100000, B00000000, B01110000,
  B11100000, B00000000, B00110000,
  B11000000, B00000000, B00110000,
  B11000000, B00000000, B00110000,
  B11000000, B00000000, B00110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B11111111, B11111111, B11110000,
  B01111111, B11111111, B11100000,
  B00011111, B11111111, B11000000,
  B00000000, B11100000, B00000000
};

const unsigned char UP[] PROGMEM = {
  B00000000, B01100000, B00000000,
  B00000000, B11110000, B00000000,
  B00000001, B11111000, B00000000,
  B00000011, B11111100, B00000000,
  B00000111, B11111110, B00000000,
  B00001111, B11111111, B00000000,
  B00011111, B11111111, B10000000,
  B00111111, B11111111, B11000000,
  B01111111, B11111111, B11100000,
  B11111111, B11111111, B11110000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000
};

const unsigned char DWN[] PROGMEM = {
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B00000111, B11111110, B00000000,
  B11111111, B11111111, B11110000,
  B01111111, B11111111, B11100000,
  B00111111, B11111111, B11000000,
  B00011111, B11111111, B10000000,
  B00001111, B11111111, B00000000,
  B00000111, B11111110, B00000000,
  B00000011, B11111100, B00000000,
  B00000001, B11111000, B00000000,
  B00000000, B11110000, B00000000,
  B00000000, B01100000, B00000000
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);   // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)


/* == SETUP == */
void setup() {
  Serial.begin(9600);

  // BUTTON CONTROL
  pinMode(btn_ON, INPUT_PULLUP);
  pinMode(btn_OFF, INPUT_PULLUP);
  pinMode(btn_thresTemp_inc, INPUT_PULLUP);
  pinMode(btn_thresTemp_dec, INPUT_PULLUP);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(12, 20);
  display.print(F("POLTEKKES"));
  display.setCursor(17, 40);
  display.print(F("SEMARANG"));

  display.display();
  delay(3000);
}


/* == LOOP == */
void loop() {
  btnCtrl();
  oled();
  delay(50);
}


/* == ADDITIONAL FUNCTION == */
void serialSEND() {
  Serial.print("Data sent: ");
  Serial.write(sendVAL);
  Serial.println("");
}

void btnCtrl() {
  if ((digitalRead(btn_ON) == LOW) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == HIGH)) {
    sendVAL = '1';
    serialSEND();
    disp_pwrON();
  }
  else if ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == LOW) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == HIGH)) {
    sendVAL = '2';
    serialSEND();
    disp_pwrOFF();
  }
  else if ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == LOW) && (digitalRead(btn_thresTemp_dec) == HIGH)) {
    sendVAL = '3';
    serialSEND();
    disp_tmpUP();
  }
  else if ((digitalRead(btn_ON) == HIGH) && (digitalRead(btn_OFF) == HIGH) && (digitalRead(btn_thresTemp_inc) == HIGH) && (digitalRead(btn_thresTemp_dec) == LOW)) {
    sendVAL = '4';
    serialSEND();
    disp_tmpDWN();
  }
}

void oled() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(5, 15);
  display.print(F("COOLER"));
  display.setCursor(5, 35);
  display.print(F("BOX"));
  display.drawBitmap(100, 5, BLOOD, 20, 50, WHITE);

  display.display();
}

void disp_pwrON() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(35, 15);
  display.println(F("COOLER BOX"));

  display.setTextSize(2);
  display.setCursor(53, 35);
  display.println(F("ON"));

  display.display();
  delay(1000);
}

void disp_pwrOFF() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(35, 15);
  display.println(F("COOLER BOX"));

  display.setTextSize(2);
  display.setCursor(48, 35);
  display.println(F("OFF"));

  display.display();
  delay(1000);
}

void disp_tmpUP() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 15);
  display.println(F("TEMPERATURE SET"));

  display.setCursor(25, 40);
  display.drawBitmap(55, 35, UP, 20, 20, WHITE);

  display.display();
  delay(1000);
}

void disp_tmpDWN() {
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 15);
  display.println(F("TEMPERATURE SET"));

  display.setCursor(25, 40);
  display.drawBitmap(55, 35, DWN, 20, 20, WHITE);

  display.display();
  delay(1000);
}
