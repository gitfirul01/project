/*
 *  Arduino: VCC -- BIRU
 *           DATA - HIJAU
 *           GND -- UNGU
 *           
 *  DS18B20: VCC -- biru muda
 *           DATA - hijau muda
 *           GND -- coklat muda
 *           
 *  FS300A: VCC -- oranye
 *          DATA - hijau
 *          GND -- putih
 */
 
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define sensorInterrupt     0       // 0 = digital pin 2 as interrupt

#define sensorPin           2       // flow meter sensor pin
#define ONE_WIRE_BUS        3       // ds18b20 pin
#define buzzer              5

#define calibrationFactor   5.5;    // the hall-effect flow sensor outputs approximately 5.5 pulses per second per litre/minute of flow.

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

volatile byte pulseCount = 0;       // variable to store pulse count from the hall-effect sensor
byte pulse1Sec = 0;                 // *variable to store pulse count

float flowRate_min = 0;             // flow in L/min
float flowRate_sec = 0;             // flow in L/sec
float vol_litres = 0;               // vol in L
float flowRate_mLsec = 0;           // flow in mL/sec
float tempC = 0;                    // temperatire in ^C

unsigned long prevTimeSensor = 0;   // variable to store previous millis time of sensor
unsigned long prevTimeBuzzer = 0;   // variable to store previous millis time of buzzer

/**/
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;

int trigState;
int lastTrigState = LOW;
/**/

bool alarm;                 // variable to store alarm condition
bool buzzerState = false;           // variable to store buzzer state
int count;                          // variable to store count of buzzer

void setup() {
  Serial.begin(115200);             // initialize serial with serial monitor
  Serial1.begin(115200);            // initialize serial with MCU
  sensors.begin();

  pinMode(buzzer, OUTPUT);
  
  pinMode(sensorPin, INPUT);        // set pinmode for sensor pin as input
  digitalWrite(sensorPin, HIGH);    // set sensor pin as input pull up

  attachInterrupt(sensorInterrupt, flowCounter, FALLING);   // set sensorInterrupt as interrupt, run flowCounter() when state is FALLING (High to Low)
}


void loop() {
  if ((millis() - prevTimeSensor) > 1000) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;
    
    flowRate_min = ( ( 1000.0 / (millis() - prevTimeSensor) ) * pulse1Sec) / calibrationFactor;    // pulseCount = 5.5 Q; Q = L/min

    prevTimeSensor = millis();

    flowRate_mLsec = flowRate_min * 1000 / 60;    // flow in mL/sec    
//    flowRate_sec = flowRate_min / 60;             // flow in L/sec
//    vol_litres += flowRate_sec;                   // vol in L

    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
        
    StaticJsonDocument<200> toMCU;
    toMCU["flowRate_mLsec"] = flowRate_mLsec;
    toMCU["tempC"] = tempC;
    serializeJson(toMCU, Serial1);

//    Serial.print("Flow rate: ");
//    Serial.print(flowRate_mLsec);
//    Serial.println(" mL/sec");
//    
//    Serial.print("Volume: ");
//    Serial.print(vol_litres);
//    Serial.println("L");
//    
//    Serial.print(tempC);
//    Serial.println(" ^C");
  }

  int reading;
  if (flowRate_mLsec > 250) {
    reading = HIGH;
  } else {
    reading = LOW;
  }
  
  if (reading != lastTrigState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != trigState) {
      trigState = reading;

      if (trigState == HIGH) {
        alarm = true;
        count = 0;
      }
    }
  }
  lastTrigState = reading;
  
  buzzerTest();
}


void flowCounter() {
  pulseCount++;         // pulse count increment
}

void buzzerTest() {
  if (((millis() - prevTimeBuzzer) > 750) && (count <= 10)) {
    if (count == 10) {
      alarm = false;
      count = 0;
      digitalWrite(buzzer, LOW);
    }
//    Serial.println(alarm);
//    Serial.println(count);
    if (alarm == true) {
      buzzerState = !buzzerState;
      if (buzzerState) {
        digitalWrite(buzzer, HIGH);
      } else {
        digitalWrite(buzzer, LOW);
      }
      count++;
    }
    prevTimeBuzzer = millis();
  }
}
