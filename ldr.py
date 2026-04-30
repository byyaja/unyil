from machine import Pin, ADC
import time

# Mendefinisikan pin yang digunakan pada ESP32
PIN_ANALOG = 34
PIN_DIGITAL = 4

# Setup ADC
adc = ADC(Pin(PIN_ANALOG))
adc.atten(ADC.ATTN_11DB)    # Range penuh 0-3.3V
adc.width(ADC.WIDTH_12BIT)  # Resolusi 12-bit (0-4095)

print("Memulai pembacaan sensor LDR...")
time.sleep(1)

# Loop utama
while True:
    # 1. Membaca Nilai Analog (Output: 0 - 4095 pada ESP32)
    nilai_cahaya = adc.read()

    # Menampilkan hasil ke Serial Monitor
    print(f"Nilai cahaya: {nilai_cahaya}", end=" | Status: ")

    # Umumnya DO bernilai 1 (HIGH) jika gelap, dan 0 (LOW) jika terang.
    # Ini bisa berbeda tergantung modul, silakan disesuaikan jika terbalik.
    if nilai_cahaya < 500:
        print("Gelap")
    elif nilai_cahaya < 1500:
        print("Redup")
    else:
        print("Terang")

    # Jeda setengah detik sebelum membaca ulang
    time.sleep(0.5)