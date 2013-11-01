# Test code to see if serial comms works between Arduino/Python
#

import serial
import time
import threading

period = 2.0							# Period to wait before taking data

ser = serial.Serial('COM6')				# Open serial port
# Print message for user
s = "Serial port " + ser.name + " opened"
print s
#

# When serial port is opened, Arduino resets. Delay to ensure commands are received properly.
time.sleep(3)	

instruction = None						# Create empty string

def measure():
#	ser.write("T")						# Send test signal to make sure the serial is working
#	print ser.readline()				# Print response for debugging

	ser.write("M")						# Send signal to take a measurement
	RH = ser.readline()					# First line output from Arduino is the relative humidity
	print RH
	temp = ser.readline()				# Second line is the temperature
	print temp

def repeat(interval, myFunction ):
	global variable
	if variable < 5:
		variable = variable + 1
		global myTimer
		myTimer = threading.Timer( interval, repeat, [interval, myFunction] )
		myTimer.start()
		myFunction()
		print variable
		
# Timer which will execute function 'measure' every 'period' seconds
# myTimer = threading.Timer(period, measure)
# myTimer.start()							# Start timer


variable = 0

# myTimer = threading.Timer(period, repeat, [period, measure, myVar] )
repeat(period, measure )

# s = "Testing - variable = " + `myVar`
# print s

def get_input():
	global instruction = raw_input("Type instructions:")

myTimer.join()
print "Timer finished"
print variable

# while 1:
	# inputThread = threading.Thread(target=get_input)
	# inputThread.join()


