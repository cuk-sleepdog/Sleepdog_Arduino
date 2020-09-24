#include <SoftwareSerial.h>

float tmp;  // 온도
int reading;  // tmp 읽어 온 값 저장
int tmpPin = A0; // 온도센서 핀 번호 지정
volatile int BPM;                   // 심박수 저장
volatile int Signal;                // 심장박동센서에서 측정되는 값 저장
volatile int IBI = 600;             // 심박수 측정 시 사용되는 시간 변수(심장이 몇초마다 뛰는지 측정)
volatile boolean Pulse = false;     // 유저의 심박수가 측정되면 True, 아무것도 측정되지 않으면 False
SoftwareSerial bluetooth(2, 3); // RX, TX -> 블루투스 시리얼 연결
const int xInput = A1; // x 좌표 
const int yInput = A2; // y 좌표
const int zInput = A3; // z 좌표 

/// 가속도 센터 변수값 세팅
int RawMin = 0;
int RawMax = 1023;
const int sampleSize = 10;


void setup()  
{
    analogReference(INTERNAL);
    interruptSetup();                 // 센서의 신호를 2ms마다 읽어 심박수로 변환하는 함수
    Serial.begin(9600);
    bluetooth.begin(9600);
}

void loop()  
{
  if(bluetooth.available())
  {

  // 체온측정
    reading = analogRead(tmpPin); // 시리얼로 받아온 값 지정
    tmp = reading / 9.31; // 온도 값 지정
    bluetooth.print("TMP : ");
    bluetooth.println(tmp);
    // 심박수 측정
    bluetooth.print("BPM : ");
    bluetooth.println(BPM);
    // 가속도 센서
   int xRaw = ReadAxis(xInput);
   int yRaw = ReadAxis(yInput);
   int zRaw = ReadAxis(zInput);
   
   float xAccel = map(xRaw, RawMin, RawMax, -3000, 3000)/1000.0;
   float yAccel = map(yRaw, RawMin, RawMax, -3000, 3000)/1000.0;
   float zAccel = map(zRaw, RawMin, RawMax, -3000, 3000)/1000.0;
  
   Serial.print("X : ");
   Serial.print(xAccel,0);
   Serial.println("G");
   Serial.print("Y : ");
   Serial.print(yAccel,0);
   Serial.print("G");
   Serial.print("Z : ");
   Serial.print(zAccel,0);
   Serial.println("G");  
   delay(1000);
  }
}

int ReadAxis(int axisPin)
{
 long reading = 0;
 analogRead(axisPin);
 delay(1);
 for (int i = 0; i < sampleSize; i++)
 {
 reading += analogRead(axisPin);
 }
 return reading/sampleSize;
}
