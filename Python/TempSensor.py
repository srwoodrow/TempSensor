#
import serial
import time
import threading

##############################################
# Global variables
##############################################
interval = 10.0								# Measurement interval (in seconds)
folder = "H:\\TempSensorTest\\"					# Folder to store log files
filename = "T_H_Log_"							# Log file name

exit = False									# Flag to tell the program when to exit
##############################################
# Function declarations
##############################################

# Function to measure temp & humidity, and write to log file
# Serial comms with Arduino (running TempSensor.ino). Sends instructions & receives data as a string.
# Creates a new file for each day, and writes time,temp,RH to file
# This is called by 'repeat' every time a measurement is required
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
			myFile = open(myFilename, 'w')
			myFile.write("Time\tT\tRH\n")
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
		
#		print(s)								# Print for debugging
		
		# Write string to file
		myFile = open(myFilename, 'a')
		myFile.write(s)
		myFile.close()
		
		lastMeasurement = time.time()			# Store time of measurement

# Wrapper function to call myFunction every myInterval seconds, until exit flag is set	
def repeat(myInterval, myFunction ):
	global exit
	if exit == False:							# Check that exit flag is not set
		global myTimer
		# Restart timer to call this same function again in myInterval seconds
		myTimer = threading.Timer( myInterval, repeat, [myInterval, myFunction] )
		myTimer.start()
		myFunction()							# Call required function
	else:
		print "Stopping timer"
		
# Function to change the measurement interval based on user input
# This is called by 'get_instruction' when the user chooses to change measurement interval
def setInterval():
	global myTimer
	myTimer.cancel()							# Stop existing timer
	
	newInterval = raw_input('Choose new interval (s):')		# Get user input
	interval = float(newInterval)				# Set new interval in global variable
	
	# Print confirmation of new interval for user
	s = "Measurement interval " + repr(interval) + "s"
	print s
	
	repeat(interval, measure )					# Start timer repeating

# Function to prompt user for instructions, read user input and interpret
def get_instruction():
	# Get user input
	instruction = raw_input('Type instructions (Q to quit, I to set new measurement interval):\n')
	if instruction == "q" or instruction == "Q":
		print("Quit command received")
		global exit
		exit = True								# Set exit flag - this will be detected next time 'repeat' is run
	elif instruction == "i" or instruction == "I":
		setInterval()							# Call function to set a new measurement interval
		
##############################################


mySerial = serial.Serial('COM6')				# Open serial port
# Print message for user
s = "Serial port " + mySerial.name + " opened"
print s

# When serial port is opened, Arduino resets. Delay to ensure commands are received properly.
time.sleep(2)	

today = time.strftime("%Y-%m-%d")				# Store today's date - need to compare to see when we need to create a new file
myFilename = folder + filename + today + ".txt" # Create string for filename
lastMeasurement = time.time()					# Store timestamp of last measurement

# Print measurement interval for user
s = "Measurement interval " + repr(interval) + "s"
print(s)

# Write header row to file initially
myFile = open(myFilename, 'w')
myFile.write("Time\tT\tRH\n")
myFile.close()

repeat(interval, measure)						# Call 'repeat' to execute 'measure' every 'interval' seconds

while exit == False:							# Repeat this until exit flag is set
	# Create new thread for receiving input, so that measurements can still be made concurrently
	inputThread = threading.Thread(target=get_instruction)
	inputThread.start()
	inputThread.join()							# Wait for thread to end (input to be received)
