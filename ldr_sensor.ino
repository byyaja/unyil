// Mendefinisikan pin yang digunakan pada ESP32
#define PIN_ANALOG 34
#define PIN_DIGITAL 4

void setup() {
  // Memulai komunikasi serial dengan baud rate 115200 (Standar ESP32)
  Serial.begin(115200);
  
  // Mengatur pin digital LDR sebagai input
  pinMode(PIN_DIGITAL, INPUT);
  
  Serial.println("Memulai pembacaan sensor LDR...");
  delay(1000);
}

void loop() {
  // 1. Membaca Nilai Analog (Output: 0 - 4095 pada ESP32)
  int nilaiCahaya = analogRead(PIN_ANALOG);
  
  // 2. Membaca Nilai Digital (Output: 0 atau 1)
  int statusGelap = digitalRead(PIN_DIGITAL);

  // Menampilkan hasil ke Serial Monitor
  Serial.print("Nilai Analog: ");
  Serial.print(nilaiCahaya);
  
  Serial.print(" | Status: ");
  // Umumnya DO bernilai 1 (HIGH) jika gelap, dan 0 (LOW) jika terang.
  // Ini bisa berbeda tergantung modul, silakan disesuaikan jika terbalik.
  if (statusGelap == HIGH) {
    Serial.println("Gelap");
  } else {
    Serial.println("Terang");
  }

  // Jeda setengah detik sebelum membaca ulang
  delay(500);
}