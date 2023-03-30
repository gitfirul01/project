/*  
 *  [ DATA CONVERSION ]
 *   
 *     x - x_min         y - y_min
 *  --------------- = ---------------
 *   x_max - x_min     y_max - y_min
 *       
 */

void setup() {
  Serial.begin(115200);
}

void loop() {
  float sensorVal = analogRead(A0);
  float voltage = sensorVal * 5 * 1000 / 1024;      // mV
  Serial.print("RAW A/D:  ");
  Serial.println(sensorVal, 0);
  
  Serial.print("Voltage:  ");
  Serial.print(voltage);
  Serial.println(" mV");

  /*
   *  example analogRead = 512.7 mV
   *  
   *   0.5127 - 0.5     P(kPa) - 0
   *  -------------- = ------------
   *     4.5 - 0.5        10 - 0
   *     
   *   512.7 - 500     1000P(kPa) - 0
   *  ------------- = ----------------
   *    4.5 - 0.5         10 - 0
   *  
   *   512.7 - 500     1000P(kPa)
   *  ------------- = ------------
   *        4             10
   *   
   *   512.7 - 500     
   *  ------------- = P(kPa)
   *       400
   *  
   *            V (mV) - 500     
   *  P(kPa) = --------------
   *               400
   *  
   *  with V (mV) is voltage read by sensor / analogRead
   */

  float pressure = (voltage - 500) / 400;
  
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println("  kPa");

  delay(500);
}
