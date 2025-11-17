int sensor[8] = {32, 33, 25, 26, 27, 14, 12, 13};
long waarde;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  for (int i = 0; i < 8; i++) {
    pinMode(sensor[i], OUTPUT);
    digitalWrite(sensor[i], HIGH);
    delayMicroseconds(10);         // laadt de condensator op
    pinMode(sensor[i], INPUT);     // zet direct terug naar input
    unsigned long start = micros();
    while (digitalRead(sensor[i]) == HIGH) {
      if (micros() - start > 3000) break; // timeout
    }
    waarde = micros() - start;            // meet tijd tot laag
    Serial.print("Waarde sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(waarde);
  }
  Serial.println();
  delay(1000);
}