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

//SD카드
#include <SPI.h>
#include <SD.h>
//RTC 센서
#include <ThreeWire.h>  
#include <RtcDS1302.h>
//SD카드 
File myFile;

//온도센서
#include <Adafruit_MLX90614.h> // 비접촉식 온도측정센서 라이브러리 불러오기 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//와이파이 모듈
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
const char* ssid = "SSID"; 
const char* password = "PASSWORD"; 
const char* host = "sleepdog.mintpass.kr:3000";
String url = "/Health/"; 


//심박수 변수
const int PulseWire = 0;      
int Threshold = 550;                                                   
PulseSensorPlayground pulseSensor; 

//수면 변수
int sleep = 0;

//RTC 변수
ThreeWire myWire(6,5,7); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);


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

  //sd카드
  Serial.print("Initializing SD card...");
 
  //SD카드 초기화 SD.begin(4) 는  CS핀번호
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }

  Rtc.Begin();
  RtcDateTime now = Rtc.GetDateTime();
  mlx.begin();


  //와이파이 모듈 셋업
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 

  Serial.println("initialization done.");
   
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void loop()
{
  const size_t capacity = JSON_OBJECT_SIZE(12);
  DynamicJsonDocument doc(capacity);

  //RTC
  RtcDateTime now = Rtc.GetDateTime();
  char datestring[11];
  char ts[6];


    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%04u/%02u/%02u"),
            now.Year(),
            now.Month(),
            now.Day());

            snprintf_P(ts, 
            countof(ts),
            PSTR("%02u:%02u"),
            now.Hour(),
            now.Minute());


    doc["DATE"]   = datestring;
    doc["TIME"]   = ts;


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
  

 
  int myBPM = pulseSensor.getBeatsPerMinute();
  
  doc["BPM"]   = 0;
if (pulseSensor.sawStartOfBeat()) {                   
  doc["BPM"]   = myBPM;
}

  String strTemp = String("");
  strTemp += (double)(mlx.readObjectTempC()); // 주변 온도를 읽습니다.
  doc["TEMP"] =strTemp;

   int nsleep = abs(acc_x)+abs(acc_y)+abs(acc_z);

  // doc["SLEEP"]   = nsleep;


  if ( (sleep - 7) <= nsleep && nsleep <= (sleep + 7))
  {
    doc["CHK"]   = "Sleep";
    sleep = nsleep;

  }

  else
  {
    doc["CHK"]   = "Wake";
    sleep = nsleep;
  }
  
 


        
 /*
  serializeJson(doc, BTSerial);
  BTSerial.println();
*/
  serializeJson(doc, Serial);
  Serial.println();



  String json;
  serializeJson(doc, json);


  myFile = SD.open("sleep.txt", FILE_WRITE);
  if (!myFile) {
    Serial.println("error opening sleep.txt");
    while (1) ;
  }
  if (myFile) {
  serializeJson(doc, myFile);
  myFile.println();

  //서버 연결 작업
  WiFiClient client;
  String address = host + url;  
  HTTPClient http;
  http.begin(address); 
  http.POST(json);
  http.end();
  myFile.close();
  }


 


 delay(1000);
   
}
