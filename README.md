# Sleepdog_Arduino

Sleepdog Project의 Aruduino Code

Server로 보내는 JSON DATA 값 

{
    
    "Product": Value, // 측정 날짜 

    "BPM":Value, // 심박수 

    "TEMP":"Value", // 체온

    "CHK":"Value" // 움직임 감지

} 



<img src="https://github.com/cuk-sleepdog/Sleepdog_Arduino/blob/master/test.png?raw=true" width="90%"></img>


변경 사항

1. 방향성 변경으로 인한 SD카드 기능 삭제 

2. RTC 센서 제거 및 서버에서 날짜 관련 객체 생성으로 변경

3. 데이터 전송 간격 최소화

4. 제품별 고유 아이디 지정화 


모든 기능 구현 완료
