#
import serial
import time
import threading

##############################################
# Global variables
##############################################
interval = 2.0									# Measurement interval (in seconds)
folder = "H:\\TempSensorTest\\"					# Folder to store log files
logName = "T_H_Log_"							# Log file name

exit = False									# Flag to tell the program when to exit
##############################################
# Function declarations
##############################################

# Function to measure temp & humidity, and write to log file
# Serial comms with Arduino (running TempSensor.ino). Sends instructions & receives data as a string.
# Creates a new file for each day, and writes time,temp,RH to file
# This is called by 'repeat' every time a measurement is required
def measure():
	global today, myFilenames, lastMeasurement, mySerialPorts
	
	myTemps = []
	myRHs = []
	
	# Only allow a measurement every 0.5s to stop errors from sensor chips
	if (time.time() - lastMeasurement) > 0.49:
		s = time.strftime("%Y-%m-%d")
		if today != s:							# If the day has changed
			today = s							# Update the day
			
			# Update filenames
			myFiles = []
			for i in range(len(mySerialPorts)):
				filename = folder + logName + mySerialPorts[i].name + "_" + today + ".txt"
				myFiles.append(filename, 'w')
			# Write header row in new files
			for file in myFiles:
				file.write("Time\tT\tRH\n")
				file.close()
		
		print(len(mySerialPorts))
		for ser in mySerialPorts:
			print("Sending command to" + ser.name)
			ser.write("M")							# Send signal to take a measurement
			myTemps.append(ser.readline())			# First line output from Arduino is the temperature
			myRHs.append(ser.readline())			# Second line is the relative humidity
			print("Readings received")
		
		print(len(myTemps))
		
		for i in range(len(myTemps)):
			myTemps[i] = myTemps[i][4:9]				# Extract temp reading only
			
			if myRHs[i][3:7] == "Error":				# Make sure there was no error
				myRHs[i] = "Error" + myRHs[i][8:]		# If so, extract error code
			else:
				myRHs[i] = myRHs[i][5:10]				# If not, extract RH reading only
			
			# Create string of tab separated timestamp, temp, RH
			s = time.strftime("%H:%M:%S")
			s += "\t" + myTemps[i] + "\t" + myRHs[i] + "\n"
			
			print(myFilenames[i])
			print(s)									# Print filename & string for debugging
			
			# Write string to file
			myFile = open(myFilenames[i], 'a')
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
	
	newInterval = raw_input('Choose new interval (s): ')		# Get user input
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


mySerialPorts = []								# Create list of serial ports

# Scan available ports and test to see if they are running the right firmware, automatically add to list.. ?
mySerialPorts.append(serial.Serial('COM5'))		# Open serial ports
mySerialPorts.append(serial.Serial('COM6'))


# For each serial port, print message for user
for ser in mySerialPorts:
	s = "Serial port " + ser.name + " opened"
	print s

# When serial port is opened, Arduino resets. Delay to ensure commands are received properly.
time.sleep(2)	

today = time.strftime("%Y-%m-%d")				# Store today's date - need to compare to see when we need to create a new file
lastMeasurement = time.time()					# Store timestamp of last measurement

myFilenames = []								# Create list of filenames
for ser in mySerialPorts:						# Create string for each filename
	myFilenames.append(folder + logName + ser.name + "_" + today + ".txt") #

# Print measurement interval for user
s = "Measurement interval " + repr(interval) + "s"
print(s)

myFiles = []									# Create list of files								

for filename in myFilenames:					# Populate list of files based on filenames
	myFiles.append(open(filename, 'w'))

for file in myFiles:							# Write header row to file initially
	file.write("Time\tT\tRH\n")
	file.close()

repeat(interval, measure)						# Call 'repeat' to execute 'measure' every 'interval' seconds

while exit == False:							# Repeat this until exit flag is set
	# Create new thread for receiving input, so that measurements can still be made concurrently
	inputThread = threading.Thread(target=get_instruction)
	inputThread.start()
	inputThread.join()							# Wait for thread to end (input to be received)
