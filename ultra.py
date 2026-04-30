from machine import Pin, time_pulse_us
import time

# Set pin
trigger = Pin(5, Pin.OUT)
echo = Pin(18, Pin.IN)

def read_distance():
    # Pastikan trigger LOW
    trigger.value(0)
    time.sleep_us(2)

    # Kirim pulse 10us
    trigger.value(1)
    time.sleep_us(10)
    trigger.value(0)

    # Baca durasi echo (microsecond)
    duration = time_pulse_us(echo, 1)

    # Hitung jarak (cm)
    distance = (duration / 2) / 29.1

    return distance

# Loop utama
while True:
    try:
        jarak = read_distance()
        print("Jarak: {:.2f} cm".format(jarak))
    except:
        print("Error membaca sensor")

    time.sleep(1)