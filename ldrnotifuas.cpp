#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "secrets.h"

// ============ KONFIGURASI WiFi ============
const char* WIFI_SSID     = WIFI_SSID_VAL;
const char* WIFI_PASSWORD  = WIFI_PASSWORD_VAL;

// ============ KONFIGURASI TELEGRAM ============
const char* TELEGRAM_BOT_TOKEN = TELEGRAM_BOT_TOKEN_VAL;
const char* TELEGRAM_CHAT_ID   = TELEGRAM_CHAT_ID_VAL;

// ============ KONFIGURASI GITHUB ============
const bool  ENABLE_GITHUB_LOG = true;
const char* GITHUB_TOKEN   = GITHUB_TOKEN_VAL;
const char* GITHUB_OWNER   = GITHUB_OWNER_VAL;
const char* GITHUB_REPO    = GITHUB_REPO_VAL;

// ============ KONFIGURASI SENSOR LDR ============
#define LDR_PIN         34  // GPIO 34 (ADC1_CH6) - Aman digunakan bersama WiFi
#define LED_INDICATOR   2   // LED onboard ESP32
const char* SENSOR_LOCATION  = "Brankas Uang LDR";

// ============ KONFIGURASI WAKTU ============
const unsigned long REPORT_INTERVAL_MS = 600000;  // 10 menit (600.000 ms)
unsigned long lastReportTime           = 0;
String lastStatus                      = "UNKNOWN";

// ============ NTP TIME ============
const char* ntpServer        = "pool.ntp.org";
const long  gmtOffset        = 28800;  // GMT+8 WITA
const int   daylightOffset   = 0;

// ============ DEKLARASI FUNGSI ============
void connectWiFi();
bool sendTelegramNotification(String status, int adcVal);
bool sendGitHubLog(String status);
String getFormattedTime();
String determineStatus(int adcVal);

// ======================================================
//                        SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("  ESP32 Sensor LDR + Telegram + GitHub");
  Serial.println("  Mode: Pemantau Cahaya Brankas (4 Level)");
  Serial.println("==========================================");

  pinMode(LDR_PIN, INPUT);
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);

  connectWiFi();

  configTime(gmtOffset, daylightOffset, ntpServer);
  Serial.println("⏰ Sinkronisasi waktu...");
  delay(2000);

  Serial.println("✅ Sistem siap! Memulai pemantauan LDR...\n");
}

// ======================================================
//                      LOOP UTAMA
// ======================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  // Baca nilai analog dari sensor LDR (0 - 4095)
  int adcVal = analogRead(LDR_PIN);
  String currentStatus = determineStatus(adcVal);

  // Set LED Indikator: menyala jika GELAP (bahaya), mati jika kondisi aman lainnya
  if (currentStatus == "Gelap") {
    digitalWrite(LED_INDICATOR, HIGH);
  } else {
    digitalWrite(LED_INDICATOR, LOW);
  }

  unsigned long now = millis();

  // Deteksi transisi instan ketika kondisi berubah dari aman menjadi GELAP (Bahaya)
  bool statusChangedToDanger = (currentStatus == "Gelap" && lastStatus != "Gelap" && lastStatus != "UNKNOWN");

  if (lastStatus == "UNKNOWN") {
    // Pembacaan awal
    lastStatus = currentStatus;
    Serial.printf("📝 Pembacaan awal LDR: %d (%s)\n", adcVal, currentStatus.c_str());
    if (ENABLE_GITHUB_LOG) {
      sendGitHubLog(currentStatus);
    }
    if (currentStatus == "Gelap") {
      sendTelegramNotification(currentStatus, adcVal);
    }
    lastReportTime = now;
  }
  else if (now - lastReportTime >= REPORT_INTERVAL_MS || statusChangedToDanger) {
    if (statusChangedToDanger) {
      Serial.printf("🚨 TRANSISI DETEKSI: Brankas Gelap (Terbuka/Ditutupi)! Nilai ADC: %d. Mengirim peringatan instan...\n", adcVal);
    } else {
      Serial.printf("⏰ Laporan berkala LDR (10 menit): %d (%s)\n", adcVal, currentStatus.c_str());
    }

    // Kirim log ke GitHub
    if (ENABLE_GITHUB_LOG) {
      bool githubOK = sendGitHubLog(currentStatus);
      if (githubOK) {
        Serial.printf("✅ GitHub: Log '%s' berhasil dikirim.\n", currentStatus.c_str());
      } else {
        Serial.println("⚠️ GitHub: Gagal mengirim log.");
      }
    }

    // Kirim Telegram hanya jika GELAP (Bahaya)
    if (currentStatus == "Gelap") {
      bool telegramOK = sendTelegramNotification(currentStatus, adcVal);
      if (telegramOK) {
        Serial.println("✅ Telegram: Notifikasi bahaya terkirim!");
      } else {
        Serial.println("❌ Telegram: Gagal mengirim notifikasi.");
      }
    }

    Serial.println();
    lastReportTime = now;
    lastStatus = currentStatus;
  }

  delay(1000); // Periksa sensor setiap 1 detik
}

// ======================================================
//              FUNGSI KLASIFIKASI CAHAYA
// ======================================================
String determineStatus(int adcVal) {
  if (adcVal >= 3000) {
    return "Gelap";
  } else if (adcVal >= 1500) {
    return "Remang-remang";
  } else if (adcVal >= 800) {
    return "Terang";
  } else {
    return "Sangat Terang";
  }
}

// ======================================================
//         ⚡ KIRIM NOTIFIKASI TELEGRAM (BAHAYA)
// ======================================================
bool sendTelegramNotification(String status, int adcVal) {
  if (status != "Gelap") {
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://api.telegram.org/bot";
  url += TELEGRAM_BOT_TOKEN;
  url += "/sendMessage";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String timestamp = getFormattedTime();

  // Pesan peringatan brankas gelap (bahaya)
  String message = "🚨 *PERINGATAN: KONDISI GELAP (BAHAYA)!*\\n";
  message += "━━━━━━━━━━━━━━━━━━━━\\n";
  message += "⚠️ Status: GELAP (Bahaya)\\n";
  message += "📊 Nilai ADC LDR: " + String(adcVal) + " (0-4095)\\n";
  message += "📍 Lokasi: " + String(SENSOR_LOCATION) + "\\n";
  message += "🕐 Waktu: " + timestamp + "\\n";
  message += "━━━━━━━━━━━━━━━━━━━━\\n";
  message += "👁️ Sensor mendeteksi ruangan gelap. Harap periksa brankas!";

  // JSON payload
  String payload = "{";
  payload += "\"chat_id\":\"" + String(TELEGRAM_CHAT_ID) + "\",";
  payload += "\"text\":\"" + message + "\",";
  payload += "\"parse_mode\":\"Markdown\"";
  payload += "}";

  int httpCode = http.POST(payload);
  http.end();

  return (httpCode == 200);
}

// ======================================================
//         📝 LOG DATA KE GITHUB DISPATCHES
// ======================================================
bool sendGitHubLog(String status) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://api.github.com/repos/";
  url += GITHUB_OWNER;
  url += "/";
  url += GITHUB_REPO;
  url += "/dispatches";

  http.begin(client, url);
  http.addHeader("Authorization", String("token ") + GITHUB_TOKEN);
  http.addHeader("Accept", "application/vnd.github.v3+json");
  http.addHeader("Content-Type", "application/json");

  String timestamp = getFormattedTime();

  String payload = "{";
  payload += "\"event_type\":\"ldr-alert\","; // Menggunakan ldr-alert
  payload += "\"client_payload\":{";
  payload += "\"status\":\"" + status + "\",";
  payload += "\"location\":\"" + String(SENSOR_LOCATION) + "\",";
  payload += "\"timestamp\":\"" + timestamp + "\"";
  payload += "}}";

  int httpCode = http.POST(payload);
  http.end();

  return (httpCode == 204);
}

// ======================================================
//              KONEKSI WiFi
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
    Serial.printf("📍 IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" GAGAL!");
    Serial.println("⚠️ Restart dalam 5 detik...");
    delay(5000);
    ESP.restart();
  }
}

// ======================================================
//              FORMAT WAKTU WIB
// ======================================================
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Waktu tidak tersedia";
  }

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S WIB", &timeinfo);
  return String(buffer);
}
