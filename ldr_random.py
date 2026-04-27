# Simulasi Sensor LDR - Python
# Jalankan: python ldr_random.py
# Hentikan: Ctrl+C

import random
import time

while True:
    nilai = random.randint(0, 4095)  # Nilai acak LDR (0 - 4095)

    print(f"Nilai LDR: {nilai}", end=" -> ")

    if nilai < 500:
        print("GELAP")
    elif nilai < 3000:
        print("REDUP")
    else:
        print("TERANG")

    time.sleep(0.5)
