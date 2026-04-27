/*
 * ESP32 - Membaca Sensor LDR (Light Dependent Resistor)
 * 
 * Rangkaian:
 *   - LDR disambungkan antara 3.3V dan pin GPIO34
 *   - Resistor 10kΩ disambungkan antara GPIO34 dan GND (voltage divider)
 * 
 *        3.3V
 *         |
 *        LDR
 *         |
 *         +-------> GPIO34 (ADC input)
 *         |
 *       10kΩ
 *         |
 *        GND
 */

// Definisi pin
const int LDR_PIN = 34;       // Pin ADC untuk LDR (GPIO34)
const int LED_PIN = 2;        // LED built-in ESP32

// Konfigurasi ADC
const int ADC_RESOLUSI = 4095; // 12-bit ADC (0 - 4095)
const int BATAS_GELAP = 1000;  // Nilai ADC di bawah ini = gelap
const int BATAS_TERANG = 3000; // Nilai ADC di atas ini = sangat terang

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  // Atur resolusi ADC ke 12-bit
  analogReadResolution(12);
  
  Serial.println("=================================");
  Serial.println("  ESP32 Sensor LDR - Siap!       ");
  Serial.println("=================================");
  Serial.println("Format: [Nilai ADC] | [Tegangan] | [Status]");
  Serial.println("-----------------------------------------");
}

void loop() {
  // Baca nilai ADC dari LDR
  int nilaiADC = analogRead(LDR_PIN);
  
  // Konversi ke tegangan (3.3V referensi)
  float tegangan = (nilaiADC / (float)ADC_RESOLUSI) * 3.3;
  
  // Konversi ke persentase kecerahan (0% = gelap, 100% = terang)
  int persentase = map(nilaiADC, 0, ADC_RESOLUSI, 0, 100);
  
  // Tentukan status cahaya
  String status;
  if (nilaiADC < BATAS_GELAP) {
    status = "GELAP";
    digitalWrite(LED_PIN, HIGH); // LED menyala saat gelap
  } else if (nilaiADC < BATAS_TERANG) {
    status = "REDUP";
    digitalWrite(LED_PIN, LOW);
  } else {
    status = "TERANG";
    digitalWrite(LED_PIN, LOW);
  }
  
  // Tampilkan hasil di Serial Monitor
  Serial.print("ADC: ");
  Serial.print(nilaiADC);
  Serial.print(" | Tegangan: ");
  Serial.print(tegangan, 2);
  Serial.print("V | Kecerahan: ");
  Serial.print(persentase);
  Serial.print("% | Status: ");
  Serial.println(status);
  
  delay(500); // Baca setiap 500ms
}
