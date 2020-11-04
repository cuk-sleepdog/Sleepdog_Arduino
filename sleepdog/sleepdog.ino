#include <ArduinoJson.h>
#include <SoftwareSerial.h>
//심박수
#define USE_ARDUINO_INTERRUPTS true    
#include <PulseSensorPlayground.h> 
//G센서
#include <avr/io.h>
#include <math.h>
#include <Wire.h>
#define I2C_ID  0x53


//RTC 센서
#include <ThreeWire.h>  
#include <RtcDS1302.h>


//온도센서
#include <Adafruit_MLX90614.h> // 비접촉식 온도측정센서 라이브러리 불러오기 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//심박수 변수
const int PulseWire = 0;      
int Threshold = 550;                                                   
PulseSensorPlayground pulseSensor; 

//수면 변수
int sleep = 0;



SoftwareSerial esp(2, 3); // RX/TX 설정, serial 객체생성


int countTrueCommand;
int countTimeCommand;
boolean found = false;

void setup()  
{

  analogReference(INTERNAL);
  Serial.begin(9600);
  esp.begin(9600);  
  

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


  mlx.begin();

  Serial.println("initialization done.");
   
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void loop()
{
  const size_t capacity = JSON_OBJECT_SIZE(9);
  DynamicJsonDocument doc(capacity);


 doc["Product"]   = 1;

 int myBPM = pulseSensor.getBeatsPerMinute();
              
 doc["Bpm"]   = myBPM;

 


  //가속도 센서 세팅 
  char str[30];
  char data[6];
  unsigned char idx = 0;
  short acc_x;
  short acc_y;
  short acc_z;


   
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
  


  String strTemp = String("");
  strTemp += (double)(mlx.readObjectTempC()); // 주변 온도를 읽습니다.
  doc["Temp"] =strTemp;

   int nsleep = abs(acc_x)+abs(acc_y)+abs(acc_z);
    




  if ( (sleep - 7) <= nsleep && nsleep <= (sleep + 7))
  {
    doc["CHK"]   = "0";
    sleep = nsleep;

  }

  else
  {
    doc["CHK"]   = "1";
    sleep = nsleep;
  }
  
 

// TCP 연결

String server = "52.231.157.24"; 
String uri = "/HealthAPI";


String output;
  serializeJson(doc, output);


  String postRequest = "POST " + uri  + " HTTP/1.1\r\n" +
                       "Host: " + server + "\r\n" +
                       "Accept: *" + "/" + "*\r\n" +
                       "Content-Length: " + output.length() + "\r\n" +
                       "Content-Type: application/json\r\n\r\n";
  postRequest+=output;
  sendCommandToESP8266("AT+CIPSTART=\"TCP\",\"" + server + "\"," + 3000, 5, "OK");
  String cipSend = "AT+CIPSEND=" + String(postRequest.length());
  sendCommandToESP8266(cipSend, 10, ">");
  sendData(postRequest);
  sendCommandToESP8266("AT+CIPCLOSE=0", 15, "OK");
  delay(100);
   
}


void sendCommandToESP8266(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp.println(command);
    if (esp.find(readReplay))
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if (found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}

void sendData(String postRequest) {
  Serial.println(postRequest);
  esp.println(postRequest);
  delay(1500);
  countTrueCommand++;
}
