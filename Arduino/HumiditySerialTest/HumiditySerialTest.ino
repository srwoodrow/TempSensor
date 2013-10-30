// Sketch to test serial comms between Arduino/Python

#include <Wire.h>       // I2C library for HIH6130 humidity sensor

#define SDAPin A4

int HIH_Address = 0x27;                // Address of HIH6130
uint16_t humidity, HIH_temp = 0;       // Current humidity & temp reading from HIH6130
byte HIH_Status = 0;                   // Status byte from the HIH6130
float rel_humidity = 0.0;              // Float to store relative humidity


void setup()
{
  Serial.begin(9600);              // Establish serial comms with PC
  
  Wire.begin();                    // Join I2C bus as master
  pinMode(SDAPin, OUTPUT);            // Set I2C data line as an output
  digitalWrite(SDAPin, HIGH);         // Turn on HIH6130
}

void loop()
{
  while(Serial.available() > 0)    // Wait until bytes are available on the serial port
  {
    char incomingByte = Serial.read();                     // Read the byte
    if(incomingByte == 'T'|| incomingByte == 't')          // Test case
    {
      Serial.println("Test command received");  
    }
    else if(incomingByte == 'M' || incomingByte == 'm')    // Signal to take a measurement
    {
      measure();
    }
  }
  
  
}


void measure()
{
  // Get humidity data from HIH6130 - error byte should be zero if no errors
  byte hum_error = fetch_humidity();
  if(hum_error == 0)      // If there were no errors
  {
    rel_humidity = (float)humidity / 16383 * 100;    // Convert raw data to relative humidity
    Serial.print("RH = ");
    Serial.println(rel_humidity);    
  }  
  else
  {
    Serial.print("RH Error ");
    Serial.println(hum_error);
  }  
}



boolean fetch_humidity()
{
  byte data[4];             // To store received data from the chip (4 bytes)          
  boolean error = false;    // Error flag to check we have received the right number of bytes (4)  
  
  Wire.beginTransmission(HIH_Address);
  Wire.endTransmission();
  delay(100);
 
  Wire.requestFrom(HIH_Address, 4);    // Request 4 bytes from the sensor
  
  // We expect 4 bytes from the chip
  for(int i = 0; i < 4; i++)
  {
    // Make sure there are bytes available (it may transmit fewer bytes than expected)
    if(Wire.available())
    {
       data[i] = Wire.read();
    } 
    else
    {
      error = true;    // Flag an error
    }
  }
  Wire.endTransmission();

  // Only process if there is no error
  if(!error)
  {
    HIH_Status = (data[0] >> 6) & B00000011;    // Extract status bits
    humidity = word( (data[0] & B00111111), data[1]);
    HIH_temp = word(data[2], data[3]);
    HIH_temp = HIH_temp >> 2;        // Drop the last 2 bits (Do Not Care)
    
    return HIH_Status;
  }
  else 
    return 4;
}
