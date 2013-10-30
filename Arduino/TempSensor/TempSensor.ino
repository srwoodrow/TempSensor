// Sketch to communicate between Arduino Uno & Honeywell HIH6130 temperature/humidity sensor chip
// Written by S.Woodrow, last updated 30/10/2013

// Communication libraries
#include <Wire.h>       // I2C library for HIH6130 humidity sensor
#include <SPI.h>        // SPI library for ADT7310 temperature sensor

// Pins used on Arduino Uno
#define SDA A4          // (I2C) Serial data line (send/receive data for humidity sensor)
#define SCK 13          // (SPI) Serial clock
#define MISO 12         // (SPI) Master In Slave Out - send data to Master (Arduino)
#define MOSI 11         // (SPI) Master Out Slave In - send data to Slave (temp sensor)
#define SS 10           // (SPI) Slave select

// Sensor SPI command bytes
#define READ_TEMP_REG            0x50  // Read from temperature register
#define READ_ID_REG              0x58  // Read from ID register
#define WRITE_TO_CONFIG_REG      0x08  // Write to configuration register
#define SIXTEEN_BIT_MODE         0x80  // Enable 16bit resolution (13bit is default)
#define ENABLE_ONE_SHOT_MODE     0x20  // Enable single-shot conversion mode (default is continuous conversion)

// Protocol for data  within the dataOut byte array
#define hum_L      0              
#define hum_H      1
#define temp_L     2
#define temp_H     3

float temperature;                     // Current temperature of the device

unsigned int conversionDelay = 250;    // Time taken to perform a temperature conversion (mS) specified min is 240mS
unsigned long lastConversionRequest;   // Keep track of time since we last performed a conversion

int HIH_Address = 0x27;                // Address of the HIH6130
unsigned int humidity = 0;             // Current humidity reading
byte HIH_Status = 0;                   // Status byte from the HIH6130

byte dataOut[4] = {0};                 // Array of bytes to send data via serial to the PC

void setup ()
{
  Serial.begin(9600);              // Establish serial comms with PC (via USB)
  
  Wire.begin();                    // Join I2C bus as master
  pinMode(SDA, OUTPUT);            // Set I2C data line as an output
  digitalWrite(SDA, HIGH);         // Turn on the HIH6130
  
  SPI.begin();                            // This sets SCK, MOSI, and SS to outputs and pulls MOSI low and SCK & CS high. 
  SPI.setDataMode(SPI_MODE3);             // (CPOL=1, CPHA=1)
  SPI.setBitOrder(MSBFIRST);              // Set bit order to MSB first
  SPI.setClockDivider(SPI_CLOCK_DIV8);    // The sensor needs a min clock period of 200ns (5MHz). Arduino Uno runs at 16MHz, so this gives 2MHz
  pinMode(MISO, INPUT);  
  
  // Make sure that no chips on the SPI bus are activated accidentally
  for (int i = 0; i < SPIbusSize; i++)
  {
    pinMode(CSPins[i], OUTPUT);           // Set CS pin as output
    digitalWrite(CSPins[i],HIGH);         // Set CS pin high (device disabled)
  }
  
  resetBus();               // Put bus into a known state (digital pins strobing on startup can confuse the sensors)
    
    
  attachInterrupt(0, measure, RISING);    // Enable Interrupt 0 (pin 2) on rising edge to take a measurement
}


void measure(){
 
  // Get humidity data from HIH6130 - error flag signals whether the correct # of bytes were received
  byte hum_error = fetch_humidity();
  fetch_temp();      // Get temperature from ADT7310 - no error flags
  
  if(hum_error == 0)
    rel_humidity = (float)humidity / 16383 * 100;    // Convert raw data to relative humidity
    
  
  

  
}

// Sends the temperature & humidity data to the PC
void sendData()
{
  
  if(hum_error == 0)
  {
   
  }
  
}

// Performs a measurement of temperature on each sensor, stores the result in global variable temperature[]
void fetch_temp()
{
  // Request conversion for each temp sensor on the SPI bus 
  for(byte i = 0; i < SPIbusSize; i++)
  {
    digitalWrite(CSPins[i], LOW);
    SPI.transfer(WRITE_TO_CONFIG_REG);
    SPI.transfer(ENABLE_ONE_SHOT_MODE | SIXTEEN_BIT_MODE);
    digitalWrite(CSPins[i], HIGH);
  }
  
  delay(conversionDelay);      // Delay to allow sensors to perform conversion
  
  // Read temperatures of each sensor on the SPI bus
  for(byte i = 0; i < SPIbusSize; i++)
  {
    digitalWrite(CSPins[i], LOW);
    SPI.transfer(READ_TEMP_REG);
    byte tempData [2];
    tempData [1] = SPI.transfer(0x00);
    tempData [0] = SPI.transfer(0x00);
    digitalWrite(CSPins[i], HIGH);

    uint16_t  tempInt = *((uint16_t*)(tempData));
    if( 0x80 & tempData[1] )      // MSB is sign bit
     temperature[i] = ((float)(tempInt - 65536))/128;  // Negative temperature
    else
      temperature[i] = ((float)tempInt)/128;      // Positive temperature
  }
  
}

void loop()
{
   float rel_humidity = 0.0;
   
   // Get data from sensor - error flag signals whether the correct # of bytes were received
   boolean error = fetch_data();

   // Make sure the correct number of bytes were received from the sensor
   if(!error)
   {
     if(HIH_Status == 0)
     {
       // Convert data to relative humidity / temp in C
       // According to formulae given in datasheet
       temp_C = ((float)temp / 16383 * 165) - 40;
       rel_humidity = (float)humidity / 16383 * 100;
       
       Serial.print("Relative humidity: ");
       Serial.print(rel_humidity);
       Serial.println("%");
       
       Serial.print("Temperature: ");
       Serial.print(temp_C);
       Serial.println(" C");
     }
     else if (HIH_Status == 1)
     {
       Serial.println("Stale data"); 
     }
     else if (HIH_Status == 2)
     {
       Serial.println("Device in command mode"); 
     }
     else if (HIH_Status == 3)
     {
       Serial.println("Diagnostic condition"); 
     }
     else
     {
       Serial.println("Unknown error");
       Serial.print("Status byte: ");
       Serial.println(HIH_Status);
     }
   }
   else
   {
     Serial.println("Error: Too few bytes received from sensor");
   }

  delay(1000);                    // Delay to take data every 1s
  
}


boolean fetch_humidity()
{
  byte da[4];             // To store received data from the chip (4 bytes)          
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
  }
  
  return error;
}
