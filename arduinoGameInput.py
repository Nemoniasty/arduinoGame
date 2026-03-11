import serial
import keyboard
import time

ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
    a = keyboard.is_pressed('a')
    d = keyboard.is_pressed('d')
    w = keyboard.is_pressed('w')
    s = keyboard.is_pressed('s')

    if a and not d:
        ser.write(b'a')
    elif d and not a:
        ser.write(b'd')
    if w and not s:
        ser.write(b'w')
    elif s and not w:
        ser.write(b's')

    if keyboard.is_pressed('I') and keyboard.is_pressed('shift'):
        ser.write(b'I')

    if keyboard.is_pressed('space'):
        ser.write(b' ')

    time.sleep(0.02)