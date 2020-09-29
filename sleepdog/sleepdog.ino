#include <ArduinoJson.h>

#include <SoftwareSerial.h>
//심박수
#define USE_ARDUINO_INTERRUPTS true    
#include <PulseSensorPlayground.h> 
//G센서
#include <avr/io.h>
#include <math.h>
#include <Wire.h>
//JSON LIB

#define I2C_ID  0x53


//체온 관련
int temperature;  
int reading;  
int lm35Pin = A2;

//심박수 변수
const int PulseWire = 0;      
int Threshold = 550;                                                   
PulseSensorPlayground pulseSensor; 
 
SoftwareSerial BTSerial(2, 3);
 
void setup()  
{

  analogReference(INTERNAL);
  Serial.begin(9600);
  BTSerial.begin(9600);  

  Wire.begin();

  // adxl345 initialize
  // power setting
  Wire.beginTransmission(I2C_ID);
  Wire.write(byte(0x2d));
  Wire.write(byte(0x18));
  Wire.endTransmission();

  // range setting
  Wire.beginTransmission(I2C_ID);
  Wire.write(byte(0x31));
  Wire.write(byte(0x02));  //0x03
  Wire.endTransmission();

  //심박수 
   pulseSensor.analogInput(PulseWire);   
  pulseSensor.setThreshold(Threshold); 

   if (pulseSensor.begin()) {
    Serial.println("Sensor OK");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
}
 
void loop()
{
  DynamicJsonDocument doc(1024);
  //가속도 센서 세팅 
  char str[30];
  char data[6];
  unsigned char idx = 0;
  short acc_x;
  short acc_y;
  short acc_z;



   reading = analogRead(lm35Pin);
   temperature = reading / 9.31;
   
    // data request
  Wire.beginTransmission(I2C_ID);
  Wire.write(byte(0x32));
  Wire.endTransmission();
  Wire.requestFrom(0x53, 6);

  idx = 0;
  while (Wire.available())
    data[idx++] = Wire.read();

  acc_x = (data[1] << 8) | data[0];
  acc_y = (data[3] << 8) | data[2];
  acc_z = (data[5] << 8) | data[4];

 
  int myBPM = pulseSensor.getBeatsPerMinute();
  doc["TEMP"] = temperature;
  doc["BPM"]   = 0;
  doc["X"]   = acc_x;
  doc["Y"]   = acc_y;
  doc["Z"]   = acc_z;
if (pulseSensor.sawStartOfBeat()) {                   
  doc["BPM"]   = myBPM;
}

 
  serializeJson(doc, BTSerial);
  BTSerial.println();

  serializeJson(doc, Serial);
  Serial.println();
  


 delay(1000);
  

   
}
