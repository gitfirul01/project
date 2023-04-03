#define Vref 3.2    // V
#define R_G  239.9  // ohm
#define Gain (1+(49400/R_G))

// float raw = 0, prev_raw = 0;
// float vol = 0, prev_vol = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int raw = analogRead(A0);
  // float vol =  raw / 1023.0 * Vref * 1000.0; // ADC val mV
  float vol =  (raw / 1023.0 * Vref * 1000.0) / Gain; // actual value mV

  // raw = ema(raw, prev_raw, 0.5);
  // vol = ema(vol, prev_vol, 0.5);

  // prev_raw = raw;
  // prev_vol = vol;
  
  // Serial.println(raw);
  Serial.println(vol);

  delay(100);
}


// float ema(float now, float prev, float alpha){
//   return alpha*now + (1-alpha)*prev;
// }
