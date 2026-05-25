/*
 * =====================================================
 *  ESP32 + Sensor LDR + WhatsApp (Fonnte) + GitHub Log
 * =====================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ============ KONFIGURASI WiFi ============
const char* WIFI_SSID      = "Byy"; //SSID WiFi
const char* WIFI_PASSWORD   = "buyyyyyy"; //Password WIfi

// ============ KONFIGURASI WHATSAPP (FONNTE) ============
const char* FONNTE_TOKEN    = "7wnBsHhqD8aiqq7rqkDk";      // Token dari fonnte.com
const char* WA_TARGET       = "6285824392015";           // Nomor tujuan (format 62xxx)

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
bool sendWhatsApp(String status, int nilaiLDR);
bool sendGitHubLog(String status, int nilaiLDR);
String getFormattedTime();

// ======================================================
//                        SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n==========================================");
  Serial.println("  ESP32 Sensor LDR");
  Serial.println("  WhatsApp (Fonnte) + GitHub Log");
  Serial.println("==========================================");

  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);

  connectWiFi();

  configTime(gmtOffset, daylightOffset, ntpServer);
  Serial.println("⏰ Sinkronisasi waktu...");
  delay(2000);

  // Baca nilai awal LDR
  int nilaiAwal = analogRead(LDR_PIN);
  Serial.printf("📊 Nilai LDR awal: %d (Threshold: %d)\n", nilaiAwal, LDR_THRESHOLD);

  // Kirim pesan startup ke WhatsApp
  sendWhatsApp("STARTUP", nilaiAwal);

  Serial.println("✅ Sistem siap! Menunggu anomali cahaya...\n");
}

// ======================================================
//                      LOOP UTAMA
// ======================================================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  int nilaiLDR = analogRead(LDR_PIN);

  Serial.printf("💡 LDR: %d | Threshold: %d | Status: %s\n",
    nilaiLDR, LDR_THRESHOLD,
    nilaiLDR > LDR_THRESHOLD ? "⚠️ ANOMALI" : "✅ NORMAL");

  // Anomali BARU terdeteksi (edge detection)
  if (nilaiLDR > LDR_THRESHOLD && !anomaliActive) {
    anomaliActive = true;
    Serial.println("\n🚨 ANOMALI TERDETEKSI!");

    digitalWrite(LED_INDICATOR, HIGH);

    // ⚡ WhatsApp via Fonnte
    bool waOK = sendWhatsApp("TERDETEKSI", nilaiLDR);
    Serial.println(waOK ? "✅ WhatsApp: Terkirim!" : "❌ WhatsApp: Gagal!");

    // 📝 GitHub Log
    if (ENABLE_GITHUB_LOG) {
      bool ghOK = sendGitHubLog("TERDETEKSI", nilaiLDR);
      Serial.println(ghOK ? "✅ GitHub: Log tersimpan." : "⚠️ GitHub: Gagal log.");
    }

    Serial.println();
  }

  // Anomali selesai (cahaya kembali normal)
  if (nilaiLDR <= LDR_THRESHOLD && anomaliActive) {
    anomaliActive = false;
    digitalWrite(LED_INDICATOR, LOW);
    Serial.println("✅ Cahaya kembali normal. Sensor siap.\n");
  }

  delay(500);
}

// ======================================================
//     📱 KIRIM NOTIFIKASI KE WHATSAPP (FONNTE)
// ======================================================
bool sendWhatsApp(String status, int nilaiLDR) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, "https://api.fonnte.com/send");
  http.addHeader("Authorization", FONNTE_TOKEN);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String timestamp = getFormattedTime();

  String message;

  if (status == "STARTUP") {
    message = "🟢 *SISTEM AKTIF*\n";
    message += "━━━━━━━━━━━━━━━━\n";
    message += "📡 ESP32 Sensor LDR menyala\n";
    message += "📍 Lokasi: " + String(SENSOR_LOCATION) + "\n";
    message += "🕐 Waktu: " + timestamp + "\n";
    message += "💡 Nilai LDR: " + String(nilaiLDR) + "\n";
    message += "━━━━━━━━━━━━━━━━\n";
    message += "✅ Siap mendeteksi anomali cahaya";
  } else {
    message = "🚨 *ANOMALI TERDETEKSI!*\n";
    message += "━━━━━━━━━━━━━━━━\n";
    message += "📡 Status: " + status + "\n";
    message += "📍 Lokasi: " + String(SENSOR_LOCATION) + "\n";
    message += "🕐 Waktu: " + timestamp + "\n";
    message += "💡 Nilai LDR: " + String(nilaiLDR) + " (Batas: " + String(LDR_THRESHOLD) + ")\n";
    message += "━━━━━━━━━━━━━━━━\n";
    message += "⚠️ Segera cek lokasi sensor!";
  }

  String postData = "target=" + String(WA_TARGET) + "&message=" + message;
  int httpCode = http.POST(postData);

  Serial.printf("📬 WhatsApp Response: %d\n", httpCode);

  http.end();
  return (httpCode == 200);
}

// ======================================================
//         📝 LOG DATA KE GITHUB
// ======================================================
bool sendGitHubLog(String status, int nilaiLDR) {
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
  payload += "\"event_type\":\"sensor-alert\",";
  payload += "\"client_payload\":{";
  payload += "\"status\":\"" + status + "\",";
  payload += "\"location\":\"" + String(SENSOR_LOCATION) + "\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"ldr_value\":\"" + String(nilaiLDR) + "\"";
  payload += "}}";

  int httpCode = http.POST(payload);
  http.end();
  return (httpCode == 204);
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
    Serial.printf("📍 IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" GAGAL!");
    Serial.println("⚠️ Restart dalam 5 detik...");
    delay(5000);
    ESP.restart();
  }
}

// ======================================================
//          FUNGSI DAPATKAN WAKTU FORMATTED
// ======================================================
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Waktu tidak tersedia";
  }

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S WITA", &timeinfo);
  return String(buffer);
}
