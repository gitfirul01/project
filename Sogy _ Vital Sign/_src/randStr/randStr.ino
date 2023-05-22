#define lenStr 5
char charArr[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
String id_;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  char id[lenStr];
  // put your main code here, to run repeatedly:
  for (int i = 0; i < lenStr; i++) {
    id[i] = charArr[random(35)];
  }
  Serial.println(random(35));
  Serial.println(String(id));
  Serial.println("");
  delay(1000);
}
