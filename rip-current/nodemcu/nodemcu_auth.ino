/*
   NodeMCU
*/

#include <FS.h>   //**

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

// 192.168.1.4

//WiFiManager wifiManager;
BlynkTimer timer;
WidgetRTC rtc;

char blynk_token[32]; // = "S5g77zqrJxwCqncwMdLXiPeezwuxAFph";
bool shouldSaveConfig = false;    //**flag for saving data
//char ssid[] = "A37f";
//char pass[] = "011235813";

float flowRate_mLsec;
//float vol_litres;
float tempC;

//**callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

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
  while (!Serial) {}

  //  WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", blynk_token, 34);   //**
  //  wifiManager.addParameter(&custom_blynk_token);    //**
  //  wifiManager.autoConnect("ripCurrent-MCU");    // add second parameters for password
  //  Blynk.config(custom_blynk_token.getValue());      //**

  //  Blynk.config(blynk_token);

  //**********************************************

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    if (SPIFFS.exists("/config.json")) {

      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");

      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);

        auto error = deserializeJson(json, buf.get());
        serializeJson(json, Serial);

        if (error) {
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(error.c_str());
          return;
        } else {
          Serial.println("\nparsed json");
          strcpy(blynk_token, json["blynk_token"]);
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  } // end read

  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);

  if (!wifiManager.autoConnect("ripCurrent-MCU")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  } else {
    Serial.println("connected!!");
  }

  //read updated parameters
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    
    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
  } //end save

  Blynk.config(blynk_token);
  bool result = Blynk.connect();

  if (result != true) {
    Serial.println("BLYNK Connection Fail");
    Serial.println(blynk_token);
    wifiManager.resetSettings();
    ESP.reset();
    delay (5000);
  } else {
    Serial.println("BLYNK Connected");
  }

  //**********************************************

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
