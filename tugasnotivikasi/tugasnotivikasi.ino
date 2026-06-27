/*
 * =====================================================
 *  ESP8266 + Sensor LDR + GitHub Log
 *  Interval: 10 menit | Durasi: 12 jam
 *  Klasifikasi: Gelap total, Remang-remang,
 *               Cahaya terang, Cahaya sangat terang
 *  Notifikasi Telegram: via GitHub Actions workflow
 * =====================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ============ KONFIGURASI WiFi ============
const char* WIFI_SSID      = "Home"; //SSID WiFi
const char* WIFI_PASSWORD   = ""; //Password WIfi


// ============ KONFIGURASI GITHUB ============
const bool  ENABLE_GITHUB_LOG  = true;
const char* GITHUB_TOKEN    = ""; // Token Github
const char* GITHUB_OWNER    = "byyaja"; // Username Github
const char* GITHUB_REPO     = "unyil"; // Repository github

// ============ KONFIGURASI SENSOR LDR ============
// ESP8266 hanya punya 1 pin analog: A0, resolusi 10-bit (0-1023)
// Nilai ADC akan di-scale ke 0-4095 agar kompatibel dengan parameter jurnal LDR
#define LDR_PIN         A0        // Pin analog untuk LDR (A0 pada ESP8266)
#define LED_INDICATOR   2         // LED bawaan ESP8266 (GPIO2)
const char* SENSOR_LOCATION = "Lampu kamar";

// Ambang batas berdasarkan jurnal LDR + kondisi remang-remang
// (nilai dalam skala 0-4095 setelah mapping dari 10-bit)
// Gelap total          : ADC >= 3763  -> BAHAYA
// Remang-remang        : 2400 <= ADC < 3763 -> AMAN
// Cahaya terang        : 1040 <= ADC < 2400 -> AMAN
// Cahaya sangat terang : ADC < 1040 -> AMAN
const int BATAS_GELAP_TOTAL   = 3763;
const int BATAS_REMANG        = 2400;
const int BATAS_CAHAYA_TERANG = 1040;

// ============ INTERVAL & DURASI ============
const unsigned long INTERVAL_BACA   = 600000;  // 10 menit (600.000 ms)
const unsigned long DURASI_TOTAL    = 43200000; // 12 jam (43.200.000 ms)
unsigned long lastReadTime   = 0;
unsigned long startTime      = 0;
bool firstRead               = true; // Agar pembacaan pertama langsung jalan

// ============ EDGE DETECTION (ANTI SPAM TELEGRAM) ============
bool gelapTotalActive = false;  // Track apakah sedang dalam kondisi gelap total

// ============ COUNTER ============
int totalBacaan   = 0;
int totalAman     = 0;
int totalBahaya   = 0;

// ============ NTP TIME ============
const char* ntpServer       = "pool.ntp.org";
const long  gmtOffset       = 28800;  // GMT+8 WIB (detik)
const int   daylightOffset  = 0;

// ============ DEKLARASI FUNGSI ============
void connectWiFi();
bool sendGitHubLog(String status, String kondisi, int nilaiLDR, bool triggerTelegram);
String getFormattedTime();
String klasifikasiCahaya(int nilaiLDR);
bool isBahaya(int nilaiLDR);
int readLDR();

// ======================================================
//                        SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("  ESP8266 Sensor LDR + GitHub Log");
  Serial.println("  Interval: 10 menit | Durasi: 12 jam");
  Serial.println("  Telegram via GitHub Actions");
  Serial.println("==========================================");

  // Setup pin
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, HIGH); // LED ESP8266 active LOW

  // Koneksi WiFi
  connectWiFi();

  // Setup NTP (waktu)
  configTime(gmtOffset, daylightOffset, ntpServer);
  Serial.println("⏰ Sinkronisasi waktu NTP...");
  delay(2000);

  // Baca nilai awal LDR
  int nilaiAwal = readLDR();
  String statusAwal = klasifikasiCahaya(nilaiAwal);
  Serial.printf("📊 Nilai LDR awal: %d | Kondisi: %s\n", nilaiAwal, statusAwal.c_str());

  // Catat waktu mulai
  startTime = millis();

  Serial.println("✅ Sistem siap! Pembacaan setiap 10 menit selama 12 jam.\n");
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║  PARAMETER SENSOR (JURNAL LDR)          ║");
  Serial.println("╠══════════════════════════════════════════╣");
  Serial.println("║  ADC >= 3763 : Gelap total    (BAHAYA)  ║");
  Serial.println("║  ADC >= 2400 : Remang-remang  (AMAN)    ║");
  Serial.println("║  ADC >= 1040 : Cahaya terang  (AMAN)    ║");
  Serial.println("║  ADC <  1040 : Cahaya sangat  (AMAN)    ║");
  Serial.println("║               terang                    ║");
  Serial.println("╚══════════════════════════════════════════╝\n");
}

// ======================================================
//                      LOOP UTAMA
// ======================================================
void loop() {
  // Cek apakah sudah melewati durasi 12 jam
  if (millis() - startTime >= DURASI_TOTAL) {
    Serial.println("\n⏱️ ====================================");
    Serial.println("  DURASI 12 JAM SELESAI!");
    Serial.printf("  Total pembacaan : %d\n", totalBacaan);
    Serial.printf("  Kondisi Aman    : %d\n", totalAman);
    Serial.printf("  Kondisi Bahaya  : %d\n", totalBahaya);
    Serial.println("====================================");
    Serial.println("💤 Sistem berhenti. Restart ESP8266 untuk sesi baru.");

    // Matikan LED
    digitalWrite(LED_INDICATOR, HIGH); // LED ESP8266 active LOW

    // Berhenti total
    while (true) {
      delay(10000);
      yield(); // Agar watchdog ESP8266 tidak reset
    }
  }

  // Pastikan WiFi tetap terhubung
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Cek apakah sudah waktunya membaca sensor (non-blocking 10 menit)
  unsigned long currentTime = millis();
  if (firstRead || (currentTime - lastReadTime >= INTERVAL_BACA)) {
    firstRead = false;
    lastReadTime = currentTime;

    // Baca nilai analog LDR (di-scale ke 0-4095)
    int nilaiLDR = readLDR();
    totalBacaan++;

    // Klasifikasi kondisi cahaya
    String kondisi = klasifikasiCahaya(nilaiLDR);
    bool bahaya = isBahaya(nilaiLDR);
    String statusLabel = bahaya ? "BAHAYA" : "AMAN";

    // Update counter
    if (bahaya) {
      totalBahaya++;
    } else {
      totalAman++;
    }

    // Hitung sisa waktu
    unsigned long elapsed = currentTime - startTime;
    unsigned long remaining = DURASI_TOTAL - elapsed;
    int jamSisa = remaining / 3600000;
    int menitSisa = (remaining % 3600000) / 60000;

    // Tampilkan di Serial Monitor
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("📖 Pembacaan ke-%d\n", totalBacaan);
    Serial.printf("💡 LDR: %d | Kondisi: %s | Status: %s\n",
      nilaiLDR, kondisi.c_str(), statusLabel.c_str());
    Serial.printf("⏳ Sisa waktu: %d jam %d menit\n", jamSisa, menitSisa);
    Serial.printf("📊 Aman: %d | Bahaya: %d\n", totalAman, totalBahaya);

    // Kontrol LED indikator (ESP8266 LED active LOW)
    if (bahaya) {
      digitalWrite(LED_INDICATOR, LOW);  // LED menyala
    } else {
      digitalWrite(LED_INDICATOR, HIGH); // LED mati
    }

    // Edge detection: trigger Telegram hanya saat MASUK ke kondisi Gelap total
    bool triggerTelegram = false;
    if (bahaya && !gelapTotalActive) {
      // Baru saja masuk kondisi gelap total
      gelapTotalActive = true;
      triggerTelegram = true;
      Serial.println("🚨 BAHAYA! Gelap total terdeteksi!");
      Serial.println("📱 Telegram akan dikirim via GitHub Actions...");
    } else if (!bahaya && gelapTotalActive) {
      // Kembali dari gelap total ke kondisi lain
      gelapTotalActive = false;
      Serial.println("✅ Cahaya kembali normal dari gelap total.");
    }

    // Kirim data ke GitHub (setiap pembacaan)
    if (ENABLE_GITHUB_LOG) {
      bool success = sendGitHubLog(statusLabel, kondisi, nilaiLDR, triggerTelegram);
      Serial.println(success ? "✅ GitHub: Data tersimpan." : "❌ GitHub: Gagal simpan.");
    }

    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  }

  yield(); // Agar watchdog ESP8266 tidak reset
}

// ======================================================
//          BACA SENSOR LDR (SCALE 10-BIT → 12-BIT)
// ======================================================
int readLDR() {
  int rawADC = analogRead(LDR_PIN);  // ESP8266: 0-1023 (10-bit)
  int scaledADC = map(rawADC, 0, 1023, 0, 4095); // Scale ke 0-4095
  return scaledADC;
}

// ======================================================
//          KLASIFIKASI KONDISI CAHAYA
// ======================================================
String klasifikasiCahaya(int nilaiLDR) {
  if (nilaiLDR >= BATAS_GELAP_TOTAL) {
    return "Gelap total";
  } else if (nilaiLDR >= BATAS_REMANG) {
    return "Remang-remang";
  } else if (nilaiLDR >= BATAS_CAHAYA_TERANG) {
    return "Cahaya terang";
  } else {
    return "Cahaya sangat terang";
  }
}

// ======================================================
//          CEK APAKAH KONDISI BAHAYA
// ======================================================
bool isBahaya(int nilaiLDR) {
  return nilaiLDR >= BATAS_GELAP_TOTAL;
}

// ======================================================
//              FUNGSI KONEKSI WiFi
// ======================================================
void connectWiFi() {
  Serial.printf("📶 Menghubungkan ke WiFi: %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" TERHUBUNG!");
    Serial.print("📍 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" GAGAL!");
    Serial.println("⚠️ Tidak bisa konek WiFi. Restart dalam 5 detik...");
    delay(5000);
    ESP.restart();
  }
}


// ======================================================
//          FUNGSI KIRIM DATA KE GITHUB API
// ======================================================
bool sendGitHubLog(String status, String kondisi, int nilaiLDR, bool triggerTelegram) {
  WiFiClientSecure client;
  client.setInsecure();  // Skip SSL verification

  HTTPClient http;

  // URL GitHub API - repository_dispatch
  String url = "https://api.github.com/repos/";
  url += GITHUB_OWNER;
  url += "/";
  url += GITHUB_REPO;
  url += "/dispatches";

  Serial.printf("📡 Mengirim ke: %s\n", url.c_str());

  http.begin(client, url);

  // Headers
  http.addHeader("Authorization", String("token ") + GITHUB_TOKEN);
  http.addHeader("Accept", "application/vnd.github.v3+json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP8266-LDR-Sensor");

  // Dapatkan waktu sekarang
  String timestamp = getFormattedTime();

  // Body JSON
  String payload = "{";
  payload += "\"event_type\":\"sensor_update\",";
  payload += "\"client_payload\":{";
  payload += "\"status\":\"" + status + "\",";
  payload += "\"kondisi\":\"" + kondisi + "\",";
  payload += "\"location\":\"" + String(SENSOR_LOCATION) + "\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"ldr_value\":\"" + String(nilaiLDR) + "\",";
  payload += "\"trigger_telegram\":\"" + String(triggerTelegram ? "true" : "false") + "\"";
  payload += "}}";

  Serial.println("📦 Payload: " + payload);

  // Kirim POST request
  int httpCode = http.POST(payload);

  Serial.printf("📬 GitHub Response: %d\n", httpCode);

  http.end();

  // GitHub API mengembalikan 204 jika berhasil
  return (httpCode == 204);
}

// ======================================================
//          FUNGSI DAPATKAN WAKTU FORMATTED
// ======================================================
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ Gagal mendapatkan waktu");
    return "Waktu tidak tersedia";
  }

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S WIB", &timeinfo);
  return String(buffer);
}
