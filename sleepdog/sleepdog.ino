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

//RTC 변수
ThreeWire myWire(6,5,7); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);


SoftwareSerial ser(2, 3); // RX/TX 설정, serial 객체생성


void setup()  
{

  analogReference(INTERNAL);
  Serial.begin(9600);
  ser.begin(9600);  
  

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




  Rtc.Begin();
  RtcDateTime now = Rtc.GetDateTime();
  mlx.begin();

  Serial.println("initialization done.");
   
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void loop()
{
  const size_t capacity = JSON_OBJECT_SIZE(9);
  DynamicJsonDocument doc(capacity);


 doc["PRODUCT"]   = 1;
  //RTC
  RtcDateTime now = Rtc.GetDateTime();
  char datestring[12];
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
  
 

// TCP 연결
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "sleepdog.mintpass.kr"; // api.thingspeak.com 접속 IP
  cmd += "\",3000";           // api.thingspeak.com 접속 포트, 80
  ser.println(cmd);
   
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
        

String output;
  serializeJson(doc, output);
  
 
  int len=output.length();
  

  
  String post = "POST /HealthAPI HTTP/1.1\r\n";
post+= "Host: sleepdog.mintpass.kr\r\nContent-Type: application/json\r\nContent-Length: ";
delay(1000);
post+= String(len)+"\r\n\r\n";
post+=output;




 
  // Send Data
  cmd = "AT+CIPSEND=";
  cmd += String(post.length());
  ser.println(cmd);
 
  if(ser.find(">")){
    ser.print(post);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("OK");
    Serial.println(post);
  }





 delay(60000);
   
}
