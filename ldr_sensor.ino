// ESP32 - Baca Sensor LDR
// Rangkaian: 3.3V -> LDR -> GPIO34 -> Resistor 10k -> GND

const int LDR_PIN = 34;  // Pin LDR

void setup() {
  Serial.begin(115200);
  Serial.println("Mulai baca LDR...");
}

void loop() {
  int nilai = analogRead(LDR_PIN);  // Baca nilai LDR (0 - 4095)

  Serial.print("Nilai LDR: ");
  Serial.print(nilai);

  if (nilai < 1000) {
    Serial.println(" -> GELAP");
  } else if (nilai < 3000) {
    Serial.println(" -> REDUP");
  } else {
    Serial.println(" -> TERANG");
  }

  delay(500);
}
