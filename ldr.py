"""
Simulasi Sensor LDR (Light Dependent Resistor) - Python
========================================================
Mensimulasikan pembacaan sensor LDR dengan nilai random.
Jalankan: python simulasi_ldr_random.py
Hentikan: Ctrl+C
"""

import random
import math
import time
import os

# =============================================
# KONFIGURASI
# =============================================
ADC_MIN = 0
ADC_MAX = 4095
BATAS_GELAP  = 1000
BATAS_TERANG = 3000
DELAY = 0.5  # detik antar pembacaan

# MODE SIMULASI:
# 1 = Acak penuh
# 2 = Perubahan halus (realistis)
# 3 = Gelombang sinusoidal
MODE_SIMULASI = 2

# =============================================
# VARIABEL STATE
# =============================================
nilai_sekarang = 2000
perubahan_maks = 300
sudut = 0.0

# =============================================
# FUNGSI
# =============================================

def get_status(nilai_adc):
    if nilai_adc < BATAS_GELAP:
        return "GELAP ", "💡 LED ON "
    elif nilai_adc < BATAS_TERANG:
        return "REDUP ", "💡 LED OFF"
    else:
        return "TERANG", "💡 LED OFF"

def baca_ldr_mode1():
    """Nilai acak penuh"""
    return random.randint(ADC_MIN, ADC_MAX)

def baca_ldr_mode2():
    """Perubahan halus / realistis"""
    global nilai_sekarang
    perubahan = random.randint(-perubahan_maks, perubahan_maks)
    nilai_sekarang += perubahan
    nilai_sekarang = max(ADC_MIN, min(ADC_MAX, nilai_sekarang))
    return nilai_sekarang

def baca_ldr_mode3():
    """Gelombang sinusoidal + noise"""
    global sudut
    nilai = int(((math.sin(sudut) + 1.0) / 2.0) * ADC_MAX)
    nilai += random.randint(-50, 50)
    nilai = max(ADC_MIN, min(ADC_MAX, nilai))
    sudut += 0.1
    if sudut > 2 * math.pi:
        sudut = 0
    return nilai

def baca_sensor():
    if MODE_SIMULASI == 1:
        return baca_ldr_mode1()
    elif MODE_SIMULASI == 2:
        return baca_ldr_mode2()
    elif MODE_SIMULASI == 3:
        return baca_ldr_mode3()

def bar_kecerahan(persen, panjang=20):
    """Buat bar visual kecerahan"""
    isi = int(persen / 100 * panjang)
    kosong = panjang - isi
    return "[" + "█" * isi + "░" * kosong + "]"

# =============================================
# MAIN
# =============================================

def main():
    nama_mode = {1: "ACAK PENUH", 2: "PERUBAHAN HALUS (Realistis)", 3: "GELOMBANG SINUSOIDAL"}

    os.system('cls' if os.name == 'nt' else 'clear')
    print("=" * 55)
    print("   🔆  ESP32 Simulasi Sensor LDR — Python          ")
    print("=" * 55)
    print(f"   Mode    : {nama_mode[MODE_SIMULASI]}")
    print(f"   Delay   : {DELAY}s | ADC Range: {ADC_MIN}–{ADC_MAX}")
    print(f"   Gelap   : < {BATAS_GELAP} | Terang: > {BATAS_TERANG}")
    print("   Hentikan: Ctrl+C")
    print("=" * 55)
    print(f"{'No':>4} | {'ADC':>5} | {'Tegangan':>8} | {'Cahaya%':>7} | {'Bar':^22} | {'Status'}")
    print("-" * 75)

    counter = 1
    try:
        while True:
            adc = baca_sensor()
            tegangan = (adc / ADC_MAX) * 3.3
            persen = round((adc / ADC_MAX) * 100)
            status, led = get_status(adc)
            bar = bar_kecerahan(persen)

            print(
                f"{counter:>4} | "
                f"{adc:>5} | "
                f"{tegangan:>6.2f} V  | "
                f"{persen:>6}% | "
                f"{bar} | "
                f"{status}  {led}"
            )

            counter += 1
            time.sleep(DELAY)

    except KeyboardInterrupt:
        print("\n" + "=" * 55)
        print("   Simulasi dihentikan. Total pembacaan:", counter - 1)
        print("=" * 55)

if __name__ == "__main__":
    main()