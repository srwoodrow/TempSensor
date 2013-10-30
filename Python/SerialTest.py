# Test code to see if serial comms works between Arduino/Python
#

import serial
import time

ser = serial.Serial('COM5')
s = "Serial port " + ser.name + " opened"
print s

time.sleep(3)

ser.write("T")
print ser.readline()

ser.write("M")
RH = ser.readline()
print RH








