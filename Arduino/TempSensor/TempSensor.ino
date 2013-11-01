// Firmware for Arduino Uno, to interface with Honeywell HIH6130 humidity/temperature sensor and ADT7310 temperature sensor


// Communication libraries
#include <Wire.h>       // I2C library for HIH6130 humidity sensor
#include <SPI.h>        // SPI library for ADT7310 temperature sensor

// Pins used on Arduino Uno
#define SDAPin A4

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


float temperature;                     // Current temperature of the ADT7310
unsigned int conversionDelay = 250;    // Time taken to perform a temperature conversion (mS) specified min is 240mS


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
  
  SPI.begin();                            // This sets SCK, MOSI, and SS to outputs and pulls MOSI low and SCK & CS high. 
  SPI.setDataMode(SPI_MODE3);             // (CPOL=1, CPHA=1)
  SPI.setBitOrder(MSBFIRST);              // Set bit order to MSB first
  SPI.setClockDivider(SPI_CLOCK_DIV8);    // The sensor needs a min clock period of 200ns (5MHz). Arduino Uno runs at 16MHz, so this gives 2MHz
  pinMode(MISO, INPUT);  
  
  // Make sure that the ADT7310 chip is disabled
  pinMode(SS, OUTPUT);                   // Set SS pin as output
  digitalWrite(SS, HIGH);                // Set SS pin high (device disabled)
  resetSPI();                            // Put bus into a known state (digital pins strobing on startup can confuse the sensors)
  
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
      delay(100);
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
  
  fetch_temp();
  Serial.print("T = ");
  Serial.println(temperature);
  
}



byte fetch_humidity()
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


void fetch_temp()
{
  // Request conversion for temp sensor 
  digitalWrite(SS, LOW);                // Enable device
  SPI.transfer(WRITE_TO_CONFIG_REG);
  SPI.transfer(ENABLE_ONE_SHOT_MODE | SIXTEEN_BIT_MODE);
  digitalWrite(SS, HIGH);               // Disable device
  
  delay(conversionDelay);               // Delay to allow sensor to perform conversion
  
  // Read temperatures of sensor
  digitalWrite(SS, LOW);                // Enable device
  SPI.transfer(READ_TEMP_REG);
  byte tempData [2];
  tempData [1] = SPI.transfer(0x00);
  tempData [0] = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);               // Disable device

  uint16_t  tempInt = *((uint16_t*)(tempData));
  if( 0x80 & tempData[1] )                           // MSB is sign bit
   temperature = ((float)(tempInt - 65536))/128;     // Negative temperature
  else
    temperature = ((float)tempInt)/128;              // Positive temperature
}


void resetSPI()      // Reset the ADT7310 on the bus by writing a series of 32 1s
{
  digitalWrite(SS, LOW);	// Enable device
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);  
  digitalWrite(SS, HIGH);       // Disable device
  delay(1);  // Make sure sensor has plenty of time to reset  
}

