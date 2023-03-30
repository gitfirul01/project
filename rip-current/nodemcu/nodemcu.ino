/*
 * NodeMCU
 */

/* JSON Library */
#include <ArduinoJson.h>
/* Blynk Library */
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
/* WiFi Manager Library */
#include <DNSServer.h>
#include <ESP8266WebServer.h> 
#include <strings_en.h>
#include <WiFiManager.h>
/* Blynk RTC Library */
#include <TimeLib.h>
#include <WidgetRTC.h>

#define BLYNK_PRINT Serial


WiFiManager wifiManager;
BlynkTimer timer;
WidgetRTC rtc;

char blynk_token[] = "S5g77zqrJxwCqncwMdLXiPeezwuxAFph";
//char ssid[] = "A37f";
//char pass[] = "011235813";

float flowRate_mLsec;
float vol_litres;
float tempC;


void clockDisplay() {
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " / " + month() + " / " + year();

  Blynk.virtualWrite(V2, currentDate);
  Blynk.virtualWrite(V3, currentTime);
}

BLYNK_CONNECTED() {
  rtc.begin();
}



void setup() {
  Serial.begin(115200);
  while(!Serial) {
  }
  wifiManager.autoConnect("ripCurrent-MCU", "0123456789");
  Blynk.config(blynk_token);

  setSyncInterval(10 * 60);
  timer.setInterval(1000L, clockDisplay);
}



void loop() {
  StaticJsonDocument<200> fromArduino;
  DeserializationError err = deserializeJson(fromArduino, Serial);
  
  flowRate_mLsec = fromArduino["flowRate_mLsec"];
  tempC = fromArduino["tempC"];
  
  Blynk.virtualWrite(V0, flowRate_mLsec);
  Blynk.virtualWrite(V1, tempC);
  
  Blynk.run();
  timer.run();
}
