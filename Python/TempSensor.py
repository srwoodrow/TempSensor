#
import serial
import time
import threading

interval = 1.0							# Measurement interval
folder = "H:\\"
filename = "T_H_Log_"


mySerial = serial.Serial('COM6')				# Open serial port
# Print message for user
s = "Serial port " + mySerial.name + " opened"
print s
#

# When serial port is opened, Arduino resets. Delay to ensure commands are received properly.
time.sleep(2)	

today = time.strftime("%Y-%m-%d")
myFilename = folder + filename + today + ".txt"
lastMeasurement = time.time()

myFile = open(myFilename, "w")
myFile.write("Time\tRH\tT\n")
myFile.close()

exit = False

def measure():
	global today, myFilename, lastMeasurement
	
	# Only allow a measurement every 0.5s to stop errors
	if (time.time() - lastMeasurement) > 0.49:
		s = time.strftime("%Y-%m-%d")
		if today != s:							# If the day has changed
			today = s							# Update the day
			# Update filename
			myFilename = folder + filename + today + ".txt"
			# Write header row in new file
			myFile = open(myFilename, "w")
			myFile.write("Time\tRH\tT")
			myFile.close()
		
		mySerial.write("M")						# Send signal to take a measurement
		temp = mySerial.readline()				# First line output from Arduino is the temperature
		RH = mySerial.readline()				# Second line is the relative humidity
		
		temp = temp[4:9]						# Extract temp reading only
		
		if RH[3:7] == "Error":					# Make sure there was no error
			RH = "Error" + RH[8:]				# If so, extract error code
		else:
			RH = RH[5:10]						# If not, extract RH reading only
		
		# Create string of tab separated timestamp, temp, RH
		s = time.strftime("%H:%M:%S")
		s += "\t" + temp + "\t" + RH + "\n"
		
		print(s)								# Print for debugging
		
		# Write string to file
		myFile = open(myFilename, "a")
		myFile.write(s)
		myFile.close()
		
		lastMeasurement = time.time()			# Store time of measurement

	
def repeat(myInterval, myFunction ):
	global exit
	if exit == False:							# Check that exit flag is not set
		global myTimer
		myTimer = threading.Timer( myInterval, repeat, [myInterval, myFunction] )
		myTimer.start()
		myFunction()
	else:
		print "Stopping timer"
		

def setInterval():
	global myTimer
	myTimer.cancel()
	
	newInterval = raw_input('Choose new interval (s):')		# Get user input
	interval = float(newInterval)		# Set new interval in global variable
	s = "Measurement interval " + repr(interval) + "s"
	print s
	

	# myTimer = threading.Timer( interval, repeat, [interval, measure] )		# Reset timer with new interval
	# myTimer.start()						# Restart timer

	repeat(interval, measure )			# Start timer repeating

	
repeat(interval, measure)	
	
def get_instruction():
	instruction = raw_input('Type instructions (Q to quit, I to set new measurement interval):\n')
	if instruction == "q" or instruction == "Q":
		print("Quit command received")
		global exit
		exit = True
	elif instruction == "i" or instruction == "I":
		setInterval()


while exit == False:
	inputThread = threading.Thread(target=get_instruction)
	inputThread.start()
	inputThread.join()
