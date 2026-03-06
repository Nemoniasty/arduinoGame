import serial
import keyboard
import time

ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
    if keyboard.is_pressed('a'):
        ser.write(b'a')
    if keyboard.is_pressed('d'):
        ser.write(b'd')
    time.sleep(0.08)  # 50ms delay to avoid flooding the serial buffer