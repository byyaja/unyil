// Mendefinisikan pin yang digunakan pada ESP32
#define PIN_ANALOG 34
#define PIN_DIGITAL 4

void setup() {
  // Memulai komunikasi serial dengan baud rate 115200 (Standar ESP32)
  Serial.begin(115200);
  
  
  Serial.println("Memulai pembacaan sensor LDR...");
  delay(1000);
}

void loop() {
  // 1. Membaca Nilai Analog (Output: 0 - 4095 pada ESP32)
  int nilaiCahaya = analogRead(PIN_ANALOG);
  

  // Menampilkan hasil ke Serial Monitor
  Serial.print("Nilai cahaya: ");
  Serial.print(nilaiCahaya);

  Serial.print(" | Status: ");
  // Umumnya DO bernilai 1 (HIGH) jika gelap, dan 0 (LOW) jika terang.
  // Ini bisa berbeda tergantung modul, silakan disesuaikan jika terbalik.
  if (nilaiCahaya < 500) {
    Serial.println("Gelap");
  } else if (nilaiCahaya < 1500) {
    Serial.println("Redup");
  } else {
    Serial.println("Terang");
  }

  // Jeda setengah detik sebelum membaca ulang
  delay(500);
}