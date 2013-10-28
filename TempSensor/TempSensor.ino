// Sketch to communicate between Arduino Uno & Honeywell HIH6130 temperature/humidity sensor chip
// Written by S.Woodrow, last updated 28/10/2013

#include <Wire.h>       // Standard I2C library

#define SDA_Pin A4

int HIH_Address = 0x27;

// Manually add a prototype to fetch_data function - this allows pass by reference
void fetch_data(byte &HIH_Status, unsigned int &humidity, unsigned int &temp);


void setup ()
{
  Serial.begin(9600);              // Establish serial comms with PC (via USB)
  Wire.begin();                    // Join I2C bus as master
  pinMode(SDA_Pin, OUTPUT);        // Set data line as an output
  digitalWrite(SDA_Pin, HIGH);     // Turn on the HIH6130
}


void loop()
{
   byte HIH_Status;
   unsigned int humidity, temp;
   
   fetch_data(HIH_Status, humidity, temp);
   
   if(HIH_Status == 0)
   {
     
   }
   
  
}


void fetch_data(byte& HIH_Status, unsigned int& humidity, unsigned int& temp)
{
  byte data[4];             // To store received data from the chip (4 bytes)
  byte HIH_status;          
  boolean error = false;
  
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
   //   Serial.Print("Error: Not enough bytes received from sensor");
      error = true;    // Flag an error
    }
  }

  // Only process if there is no error
  if(!error)
  {
    HIH_status = (data[0] >> 6) & B00000011;    // Extract status bits
    humidity = word( (data[0] & B00111111), data[1]);
    temp = word(data[2], data[3]);
    temp = temp >> 2;        // Drop the last 2 bits (Do Not Care)
  }
  
  

}
