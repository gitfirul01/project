#define Vref 3.2                  // V
#define R_G 239.9                 // ohm
#define Gain (1 + (49400 / R_G))  // https://www.analog.com/media/en/technical-documentation/data-sheets/ad8221.pdf

float raw = 0;//, prev_raw = 0;
float vol = 0;//, prev_vol = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  raw = analogRead(A0);
  // vol = raw / 1023.0 * Vref * 1000.0;           // ADC val mV
  vol = (raw / 1023.0 * Vref * 1000.0) / Gain;  // actual val mV

  // ema(&raw, &prev_raw, 0.5);
  // ema(&vol, &prev_vol, 0.5);

  // Serial.println(raw);
  Serial.println(vol);

  delay(10);
}

// void ema(float* now, float* prev, float alpha) {
//   now = alpha * now + (1 - alpha) * prev;
//   prev = now;
// }
