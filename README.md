# 📡 ESP8266 Sensor LDR - Monitor Cahaya

> Sistem monitoring intensitas cahaya menggunakan ESP8266 + Sensor LDR
> Notifikasi via Telegram | Data tersimpan di GitHub | Dashboard Interaktif

## 📊 Status Terakhir

| Item | Detail |
|------|--------|
| 🔔 Status | **AMAN** |
| 💡 Kondisi | **Cahaya terang** |
| 📍 Lokasi | Lampu kamar |
| 🕐 Waktu | 2026-06-27 21:37:33 WIB |
| 📈 Nilai LDR | 1069 |
| 📁 Total Log | 31 data |

## 📂 File Log

Data lengkap di [`logs/sensor_log.csv`](logs/sensor_log.csv)

## 🌐 Dashboard

Buka [`index.html`](index.html) untuk melihat dashboard interaktif

## 🏗️ Arsitektur

```
[Sensor LDR] → [ESP8266] → [GitHub API] → [GitHub Actions] → [Telegram Bot]
                                                            → [Log CSV]
                                                            → [Dashboard]
```

## 📋 Parameter Sensor (Jurnal LDR)

| No | Kondisi Cahaya | Nilai ADC | Status | Keterangan |
|----|----------------|-----------|--------|------------|
| 1 | Gelap total | ≥ 3763 | 🔴 BAHAYA | Intensitas cahaya sangat rendah |
| 2 | Remang-remang | 2400 - 3762 | 🟢 AMAN | Cahaya redup/samar |
| 3 | Cahaya terang | 1040 - 2399 | 🟢 AMAN | Cahaya cukup terang |
| 4 | Cahaya sangat terang | < 1040 | 🟢 AMAN | Paparan cahaya tinggi |

---
*Diperbarui otomatis pada 2026-06-27 21:37:33 WIB*
