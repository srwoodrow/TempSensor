// Sketch to test serial comms between Arduino/Python

#define LED 13

void setup()
{
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
   
}

void loop()
{
  while(Serial.available() > 0)
  {
    for(int i = 0; i < 10; i++)
    {
      digitalWrite(LED, HIGH);
      delay(100);
    }
    
    char incomingByte = Serial.read();
    if(incomingByte == 'T')
    {
      digitalWrite(LED, HIGH);
      delay(1000);
      Serial.println("Command received");  
      digitalWrite(LED, LOW);
    }
    
  } 
}
