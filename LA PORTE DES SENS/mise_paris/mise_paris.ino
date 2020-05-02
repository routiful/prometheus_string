/*
 * ESP8266 강좌 시리즈
 * 미제먼지 데이터 받아오기 3편
 * 
 * 본 저작물은 '한국환경공단'에서 실시간 제공하는 '한국환경공단_대기오염정보'를 이용하였습니다.
 * https://www.data.go.kr/dataset/15000581/openapi.do
 */

#include <ESP8266WiFi.h> // ESP 8266 와이파이 라이브러리
#include <ESP8266HTTPClient.h> // HTTP 클라이언트
#include <Adafruit_NeoPixel.h> // 네오픽셀 라이브러리
#define NUMPIXELS      30 // 네오픽셀 LED 수
#define LED_PIN        D2 // 네오픽셀 입력 핀
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); // 네오픽셀 객체

String url = "http://www.airparif.asso.fr/services/api/1.1/indiceJour?date=jour";
//http://www.airparif.asso.fr/en/stations/index

int no2, o3, pm10 = 0; // 대기오염정보 데이터값
int score = 0; // 대기오염점수 0-최고 7-최악

void setup()
{
  // 시리얼 세팅
  Serial.begin(9600);
  while(!Serial);

  // 네오픽셀 초기화
  pixels.begin();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();
//  pixels.setBrightness(50);

  // 와이파이 접속
  WiFi.begin("AtelierVU", "vunet0401"); // 공유기 이름과 비밀번호

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) // 와이파이 접속하는 동안 "." 출력
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP()); // 접속된 와이파이 주소 출력
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) // 와이파이가 접속되어 있는 경우
  {
    WiFiClient client; // 와이파이 클라이언트 객체
    HTTPClient http; // HTTP 클라이언트 객체

    if (http.begin(url)) {  // HTTP
      // 서버에 연결하고 HTTP 헤더 전송
      int httpCode = http.GET();

      // httpCode 가 음수라면 에러
      if (httpCode > 0) { // 에러가 없는 경우
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString(); // 받은 XML 데이터를 String에 저장
          Serial.printf("%s\n", payload.c_str());
          o3 = getNumber(payload, "o3", 14);
          no2 = getNumber(payload, "no2", 15);
          pm10 = getNumber(payload, "pm10", 16);
        }
      } else {
        Serial.printf("[HTTP] GET... Failed. Connection Failed: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP] Access denied\n");
    }
    score = getScore(); // score 변수에 대기오염점수 저장
    Serial.println(score); // 시리얼로 출력
    setLEDColor(score); // 점수에 따라 LED 색상 출력
    delay(600000);
  }
}

int getNumber(String str, String tag, int offset) {
  int f = str.indexOf(tag) + offset;
  int t = str.indexOf(",", f); // 다음 테그시작위치
  String num = str.substring(f, t); // 테그 사이의 숫자를 문자열에 저장
  Serial.printf("%s\n", num.c_str());
  return num.toInt(); // 문자를 정수로 변환 후 반환
}

int getScore() {
  int s = (pm10 + o3 + no2) / 3;
  if (s >= 100)
    s = 7;
  else if (s >= 75 && s < 100)
    s = 6;
  else if (s >= 50 && s < 75)
    s = 5;
  else if (s >= 25 && s < 50)
    s = 3;
  else if (s > 0 && s < 25)
    s = 1;
  else 
    s = 0;
    
  return s;
}

void setLEDColor(int s) {
  int color;
  if (s == 0) // 최고
    color = pixels.Color(0, 63, 255);
  else if (s == 1) // 좋음
    color = pixels.Color(0, 127, 255);
  else if (s == 2) // 양호
    color = pixels.Color(0, 255, 255);
  else if (s == 3) // 보통
    color = pixels.Color(0, 255, 63);
  else if (s == 4) // 나쁨
    color = pixels.Color(255, 127, 0);
  else if (s == 5) // 상당히 나쁨
    color = pixels.Color(255, 63, 0);
  else if (s == 6) // 매우 나쁨
    color = pixels.Color(255, 31, 0);
  else // 최악
    color = pixels.Color(255, 0, 0);
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}
