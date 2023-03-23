#define vcc 5.0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  float val = analogRead(A0) / 1023.0 * vcc * 1000.0;   // mV
  Serial.println(val);
  delay(500);
}
