# Test code to see if serial comms works between Arduino/Python
#

import serial
import time

ser = serial.Serial('COM5')
s = "Serial port " + ser.name + " opened"
print s

# When serial port is opened, Arduino resets. Delay to ensure commands are received properly.
time.sleep(3)				

ser.write("T")				# Send test signal to make sure the serial is working
print ser.readline()		# Print response for debugging

ser.write("M")				# Send signal to take a measurement
RH = ser.readline()			# First line output from Arduino is the relative humidity
print RH
temp = ser.readline()		# Second line is the temperature
print temp







