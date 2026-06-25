# 📡 ESP32 Sensor LDR - Log Anomali Cahaya

> Sistem deteksi anomali cahaya menggunakan ESP32 + Sensor LDR
> Notifikasi via Telegram | Data tersimpan di GitHub

## 📊 Status Terakhir

| Item | Detail |
|------|--------|
| 🔔 Status | **TEST** |
| 📍 Lokasi | Lampu kamar |
| 🕐 Waktu | Manual Test |
| 💡 Nilai LDR | 2500 |
| 📁 Total Log | 21 data |

## 📂 File Log

Data lengkap di [`logs/sensor_log.csv`](logs/sensor_log.csv)

## 🏗️ Arsitektur

```
[Sensor LDR] → [ESP32] → [GitHub API] → [GitHub Actions] → [Telegram Bot]
                                                         → [Log CSV]
```

---
*Diperbarui otomatis pada Manual Test*
