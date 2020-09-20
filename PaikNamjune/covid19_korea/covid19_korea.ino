#include <ESP8266WiFi.h> // ESP 8266 와이파이 라이브러리
#include <ESP8266HTTPClient.h> // HTTP 클라이언트
#include <Adafruit_NeoPixel.h> // 네오픽셀 라이브러리
#define NUMPIXELS      30 // 네오픽셀 LED 수
#define LED_PIN        D4 // 네오픽셀 입력 핀
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); // 네오픽셀 객체

String start_create_date = "20200901";
String end_create_date = "20201231";
String num_of_rows = "1";
String page_no = "1";
String key = "TXb7Lz0%2FjL91wRXXEvNRe5OIyQkWO3wEC%2BgNcAdFMFegX%2Fhlj3pjm9%2BHrHI1ph8v9KZ1f7nLwp3waGxkKj01Yw%3D%3D";
String url = "http://openapi.data.go.kr/openapi/service/rest/Covid19/getCovid19InfStateJson?serviceKey=" + key + "&pageNo=" + page_no + "&numOfRows=" + num_of_rows + "&startCreateDt=" + start_create_date + "&endCreateDt=" + end_create_date + "&";

//float so2, co, o3, no2, pm10, pm25 = 0; // 대기오염정보 데이터값
//int score = 0; // 대기오염점수 0-최고 7-최악

int decide_cnt = 0; // 확진자 수
int clear_cnt = 0; // 격리해제 수
int exam_cnt = 0; // 검사진행 수
int death_cnt = 0; // 사망자 수
int care_cnt = 0; // 치료중 환자 수
int resutl_neg_cnt = 0; // 결과 음성 수
int acc_exam_cnt = 0; // 누적 검사 수
int acc_exam_comp_cnt = 0; // 누적 검사 완료 수

int former_decide_cnt = 0; // 전날 확진자 수

int diff_decide_cnt = 0;

int state = 0; // covid19 state

void setup()
{
  // 시리얼 세팅
  Serial.begin(9600);
  // while(!Serial);

  // 네오픽셀 초기화
  pixels.begin();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();
//  pixels.setBrightness(50);


  // 와이파이 접속
  WiFi.begin("3Com5", ""); // 공유기 이름과 비밀번호

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

void loop() 
{
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
          int today_index = payload.indexOf("<item>");
          decide_cnt = getNumber(payload, "<decideCnt>", today_index);
          clear_cnt = getNumber(payload, "<clearCnt>", today_index);
          exam_cnt = getNumber(payload, "<examCnt>", today_index);
          death_cnt = getNumber(payload, "<deathCnt>", today_index);
          care_cnt = getNumber(payload, "<careCnt>", today_index);
          resutl_neg_cnt = getNumber(payload, "<resutlNegCnt>", today_index);
          acc_exam_cnt = getNumber(payload, "<accExamCnt>", today_index);
          acc_exam_comp_cnt = getNumber(payload, "<accExamCompCnt>", today_index);
          
          Serial.print(" 확진자 "); Serial.println(decide_cnt);
          Serial.print(" 격리해제 "); Serial.println(clear_cnt);
          Serial.print(" 검사진행 "); Serial.println(exam_cnt);
          Serial.print(" 사망자 "); Serial.println(death_cnt);
          Serial.print(" 치료중 환자 "); Serial.println(care_cnt);
          Serial.print(" 결과 음성 "); Serial.println(resutl_neg_cnt);
          Serial.print(" 누적 검사 "); Serial.println(acc_exam_cnt);
          Serial.print(" 누적 검사 완료 "); Serial.println(acc_exam_comp_cnt);

          int yesterday_index = payload.indexOf("</item>");
          former_decide_cnt = getNumber(payload, "<decideCnt>", yesterday_index);
          Serial.println("");
          Serial.print(" 전날 확진자 "); Serial.println(former_decide_cnt);

          diff_decide_cnt = decide_cnt - former_decide_cnt;
          Serial.print(" 확진자 증가 "); Serial.println(diff_decide_cnt);
        }
      } else {
        Serial.printf("[HTTP] GET... Failed. Connection Failed: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP] Access denied\n");
    }
    state = getCovid19State();
    Serial.println(state); // 시리얼로 출력
    setLEDColor(state); // 상태에 따라 LED 색상 출력
    
    delay(1000 * 60 * 6);
  }
}

int getNumber(String str, String tag, int from) {
  int f = str.indexOf(tag, from) + tag.length(); // 태그의 위치와 태그의 문자 길이의 합
  int t = str.indexOf("<", f); // 다음 태그시작위치
  String s = str.substring(f, t); // 태그 사이의 숫자를 문자열에 저장
  return s.toInt(); // 문자를 정수로 변환 후 반환
}

int getCovid19State() {
  int state = diff_decide_cnt / 10;
  return state;
}

void setLEDColor(int s) {
  int color;
  if (s == 0) // 아주 좋음
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
    color = pixels.Color(255, 33, 0);
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}
