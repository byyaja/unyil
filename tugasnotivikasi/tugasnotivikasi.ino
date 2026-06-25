/*
 * =====================================================
 *  ESP32 + Sensor LDR + tELE + GitHub Log
 * =====================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ============ KONFIGURASI WiFi ============
const char* WIFI_SSID      = "py"; //SSID WiFi
const char* WIFI_PASSWORD   = "11111111"; //Password WIfi

// ============ KONFIGURASI GITHUB ============
const bool  ENABLE_GITHUB_LOG  = true;
const char* GITHUB_TOKEN    = "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"; // Token Github
const char* GITHUB_OWNER    = "byyaja"; // Username Github
const char* GITHUB_REPO     = "unyil"; // Repository github

// ============ KONFIGURASI SENSOR LDR ============
#define LDR_PIN         34        // Pin analog untuk LDR (GPIO 34)
#define LED_INDICATOR   2         // LED bawaan ESP32
const int LDR_THRESHOLD = 2000;  // Ambang batas deteksi (sesuaikan!)
                                   // > threshold = gelap/terhalang = anomali
                                   // < threshold = normal/terang
const char* SENSOR_LOCATION = "Lampu kamar";

// ============ EDGE DETECTION (ANTI DOUBLE) ============
bool anomaliActive = false;

// ============ NTP TIME ============
const char* ntpServer       = "pool.ntp.org";
const long  gmtOffset       = 28800;  // GMT+8 WITA detik
const int   daylightOffset  = 0;

// ============ DEKLARASI FUNGSI ============
void connectWiFi();
bool sendGitHubAlert(String status, int nilaiLDR);
String getFormattedTime();

// ======================================================
//                        SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("  ESP32 Sensor LDR + GitHub + Telegram");
  Serial.println("==========================================");

  // Setup pin
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);
  // LDR di GPIO 34 tidak perlu pinMode (analog input default)

  // Koneksi WiFi
  connectWiFi();

  // Setup NTP (waktu)
  configTime(gmtOffset, daylightOffset, ntpServer);
  Serial.println("⏰ Sinkronisasi waktu NTP...");
  delay(2000);

  // Baca nilai awal LDR
  int nilaiAwal = analogRead(LDR_PIN);
  Serial.printf("📊 Nilai LDR awal: %d (Threshold: %d)\n", nilaiAwal, LDR_THRESHOLD);

  Serial.println("✅ Sistem siap! Menunggu anomali cahaya...\n");
}

// ======================================================
//                      LOOP UTAMA
// ======================================================
void loop() {
  // Pastikan WiFi tetap terhubung
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Baca nilai analog LDR (0 - 4095)
  int nilaiLDR = analogRead(LDR_PIN);

  // Tampilkan nilai LDR di Serial Monitor
  Serial.printf("💡 LDR: %d | Threshold: %d | Status: %s\n",
    nilaiLDR, LDR_THRESHOLD,
    nilaiLDR > LDR_THRESHOLD ? "⚠️ ANOMALI" : "✅ NORMAL");

  // DETEKSI: Nilai LDR melewati threshold (gelap/terhalang)
  if (nilaiLDR > LDR_THRESHOLD && !anomaliActive) {
    // Anomali BARU terdeteksi (edge detection)
    anomaliActive = true;
    Serial.println("\n🚨 ANOMALI CAHAYA TERDETEKSI!");

    // Nyalakan LED indikator
    digitalWrite(LED_INDICATOR, HIGH);

    // Kirim alert ke GitHub → Telegram
    bool success = sendGitHubAlert("TERDETEKSI", nilaiLDR);

    if (success) {
      Serial.println("✅ Alert berhasil dikirim ke GitHub!");
      Serial.println("📱 Notifikasi Telegram akan segera muncul...\n");
    } else {
      Serial.println("❌ Gagal mengirim alert!\n");
    }
  }

  // Anomali selesai (cahaya kembali normal)
  if (nilaiLDR <= LDR_THRESHOLD && anomaliActive) {
    anomaliActive = false;
    digitalWrite(LED_INDICATOR, LOW);
    Serial.println("✅ Cahaya kembali normal. Sensor siap.\n");
  }

  delay(500);  // Baca sensor setiap 0.5 detik
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
//          FUNGSI KIRIM ALERT KE GITHUB API
// ======================================================
bool sendGitHubAlert(String status, int nilaiLDR) {
  WiFiClientSecure client;
  client.setInsecure();  // Skip SSL verification (untuk kemudahan)

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
  http.addHeader("User-Agent", "ESP32-Device"); // Tambahan: User-Agent wajib untuk GitHub API

  // Dapatkan waktu sekarang
  String timestamp = getFormattedTime();

  // Body JSON (event_type diubah menjadi "sensor_update")
  String payload = "{";
  payload += "\"event_type\":\"sensor_update\","; // Diubah dari "sensor-alert" ke "sensor_update"
  payload += "\"client_payload\":{";
  payload += "\"status\":\"" + status + "\",";
  payload += "\"location\":\"" + String(SENSOR_LOCATION) + "\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"ldr_value\":\"" + String(nilaiLDR) + "\"";
  payload += "}}";
  Serial.println("📦 Payload: " + payload);

  // Kirim POST request
  int httpCode = http.POST(payload);

  Serial.printf("📬 HTTP Response Code: %d\n", httpCode);

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
