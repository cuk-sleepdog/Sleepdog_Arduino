#include <SoftwareSerial.h>

float tmp;  // 온도
int reading;  // tmp 읽어 온 값 저장
int tmpPin = A0; // 온도센서 핀 번호 지정
volatile int BPM;                   // 심박수 저장
volatile int Signal;                // 심장박동센서에서 측정되는 값 저장
volatile int IBI = 600;             // 심박수 측정 시 사용되는 시간 변수(심장이 몇초마다 뛰는지 측정)
volatile boolean Pulse = false;     // 유저의 심박수가 측정되면 True, 아무것도 측정되지 않으면 False

SoftwareSerial bluetooth(2, 3); // RX, TX -> 블루투스 시리얼 연결

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
    bluetooth.print("BPM: ");
    bluetooth.println(BPM);
    delay(1000);
  }
}
