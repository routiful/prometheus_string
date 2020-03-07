/* Authors: Darby Lim */

#include <DynamixelShield.h>
DynamixelShield dxl;

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)
  #include <SoftwareSerial.h>
  SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
  #define DEBUG_SERIAL soft_serial
#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_SAM_ZERO)
  #define DEBUG_SERIAL SerialUSB
#else
  #define DEBUG_SERIAL Serial
#endif

// #define GET_MOTION
 #define PLAY_MOTION

const int32_t DXL_BAUDRATE = 1000000;
const float DXL_PROTOCOL_VERSION = 1.0;

const uint8_t DXL_CNT = 4;
const uint8_t DXL_ID[DXL_CNT] = {1, 2, 3, 4};

const uint8_t MOVE_TIME_CNT = 1;
const uint8_t DELAY_TIME_CNT = 1;

uint8_t page_cnt_ = 0;
uint32_t delay_t_ = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t PAGE = 2;
const float motion_[PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {
  // deg ... move_time(sec)(From n-1 to n), delay_time(sec)(Fron n to n+1)
  {150.00, 150.00, 150.0, 0.29, 3.0, 5.0},
  {180.00, 120.00, 180.0, 0.29, 1.0, 0.0}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float CONTROL_PERIOD = 0.050; // sec

class Ultrasonic
{
 public:
  Ultrasonic(){}
  ~Ultrasonic(){}

  init(int set_trig_pin, int set_echo_pin)
  {
    trig_pin = set_trig_pin;
    echo_pin = set_echo_pin;

    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
  }

  float get_distance()
  {
    float duration, distance;
    digitalWrite(trig_pin, HIGH);
    delay(10);
    digitalWrite(trig_pin, LOW);
   
    // echoPin 이 HIGH를 유지한 시간을 저장 한다.
    duration = pulseIn(echo_pin, HIGH);
    // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
    // 340은 초당 초음파(소리)의 속도, 10000은 밀리세컨드를 세컨드로, 왕복거리이므로 2로 나눠준다.
    distance = ((float)(340 * duration) / 10000) / 2;
  
    return distance;
  }

  float get_filtered_distance()
  {
    return average_filter(get_distance());
  }

 private:
  float average_filter(const float dist)
  {
    const int numReadings = 5;
    static int readings[numReadings];      // the readings from the analog input
    static int readIndex = 0;     // the index of the current reading
    static int total = 0;         // the running total
    int average = 0;              // the average
  
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = dist;
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;
  
    // if we're at the end of the array...
    if (readIndex >= numReadings) 
    {
      // ...wrap around to the beginning:
      readIndex = 0;
    }
  
    // calculate the average:
    return average = total / numReadings;
  }
  
  int trig_pin;
  int echo_pin;
};

Ultrasonic ultrasonic;

class RGBLED
{
 public:
    RGBLED()
    {

    }
    ~RGBLED(){}

    init(int R_pin, int G_pin, int B_pin)
    {
      r_pin = R_pin;
      g_pin = G_pin;
      b_pin = B_pin;
      
      pinMode(r_pin,OUTPUT);
      pinMode(g_pin,OUTPUT);
      pinMode(b_pin,OUTPUT);
    }

    on(int R, int G, int B)
    {
      analogWrite(r_pin, R);
      analogWrite(g_pin, G);
      analogWrite(b_pin, B);
    }
    
    off()
    {
      
    }

 private:
  int r_pin;
  int g_pin;
  int b_pin;
};

RGBLED rgb_led;

void setup() 
{
  DEBUG_SERIAL.begin(57600);
  while(!DEBUG_SERIAL);

  dxl.begin(DXL_BAUDRATE);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int id = DXL_ID[0]; id <= DXL_ID[DXL_CNT-1]; id++)
  {
    dxl.ping(id);
    dxl.torqueOff(id);
    dxl.setOperatingMode(id, OP_POSITION);
    dxl.torqueOn(id);
#ifdef GET_MOTION
    dxl.torqueOff(id);
#endif
  }

  rgb_led.init(9, 10, 11);
  ultrasonic.init(11, 12);
  
  DEBUG_SERIAL.println("Ready to Start");
}

void move()
{  
  const float DEGPERSEC_TO_RPM = 0.17f;
  const float DXL_AX12_RPM_UNIT = 0.111f;
  const uint8_t MOVE_TIME = DXL_CNT + MOVE_TIME_CNT - 1;
  const uint8_t DELAY_TIME = DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1;
  
  float diff_deg;
  float deg_per_sec;
  float rpm_per_sec;

  static uint32_t delay_t = millis();

  if (dxl.readControlTableItem(MOVING, DXL_ID[0]) == 1)
  {
    delay_t = millis();
    return;
  }
  else
  {
    if ((millis()-delay_t) >= (motion_[page_cnt_][DELAY_TIME] * 1000))
    {
      page_cnt_++;
      if (page_cnt_ >= PAGE)
      {
        page_cnt_ = 0;
      }
    }
    else
    {
      return;
    }
  }
  
  for (uint8_t id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
  {
    diff_deg = dxl.getPresentPosition(id, UNIT_DEGREE) - motion_[page_cnt_][num];
    deg_per_sec = fabs(diff_deg) / motion_[page_cnt_][MOVE_TIME];
    rpm_per_sec = deg_per_sec * DEGPERSEC_TO_RPM / DXL_AX12_RPM_UNIT;

//    DEBUG_SERIAL.print("id "); DEBUG_SERIAL.print(id);
//    DEBUG_SERIAL.print(" pos "); DEBUG_SERIAL.print(fabs(diff_deg));
//    DEBUG_SERIAL.print(" deg_per_sec "); DEBUG_SERIAL.print(deg_per_sec);
//    DEBUG_SERIAL.print(" rpm_per_sec "); DEBUG_SERIAL.print(rpm_per_sec);
//    DEBUG_SERIAL.print(" page_cnt "); DEBUG_SERIAL.print(page_cnt_);
    
    dxl.writeControlTableItem(MOVING_SPEED, id, (uint32_t)rpm_per_sec);
    dxl.setGoalPosition(id, motion_[page_cnt_][num], UNIT_DEGREE);
  }
}

void loop()
{
#ifdef PLAY_MOTION
  static uint32_t t = millis();
  if ((millis() - t) >= (CONTROL_PERIOD * 1000))
  {
    if (page_cnt_%3 == 0)
    {
      rgb_led.on(255, 0, 0);
    }
    else if (page_cnt_%3 == 1)
    {
      rgb_led.on(0, 255, 0);
    }
    else if (page_cnt_%3 == 2)
    {
      rgb_led.on(0, 0, 255);
    }

    DEBUG_SERIAL.print("Ultra : ");
    DEBUG_SERIAL.println(ultrasonic.get_distance());

    move();
    
    t = millis();
  }
#endif

#ifdef GET_MOTION
  get_motion();
#endif

}

void get_motion()
{
  const uint8_t STRING_BUF_NUM = 10;
  String cmd[STRING_BUF_NUM];

  static uint8_t page = 0;
  float motion[20][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT];
  
  if (DEBUG_SERIAL.available() > 0)
  {
    String str = DEBUG_SERIAL.readStringUntil('\n');

    if (str == "h" || str == "help" || str == "?")
    {
      DEBUG_SERIAL.println("Press 'g' to save positions of Dynamixels");
      DEBUG_SERIAL.println("Press 'e' to get a code that has a motion array");
    }
    else if (str == "g")
    {
      DEBUG_SERIAL.print("Page ");
      DEBUG_SERIAL.print(page);
      
      for (int id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
      {
        motion[page][num] = dxl.getPresentPosition(id, UNIT_DEGREE);
        DEBUG_SERIAL.print(" | ");
        DEBUG_SERIAL.print(motion[page][num]);
      }
      page++;
      DEBUG_SERIAL.print("\n");
    }
    else if (str == "d")
    {
      page--;
    }
    else if (str == "e" || page >= 250)
    { 
      DEBUG_SERIAL.print("const uint8_t PAGE = ");
      DEBUG_SERIAL.print(page);
      DEBUG_SERIAL.println(";");

      DEBUG_SERIAL.println("const float motion_[PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {");
      DEBUG_SERIAL.println("\t// deg ... move_time(sec), delay_time(sec)");
      
      for (int num = 0; num < page; num++)
      {
        DEBUG_SERIAL.print("\t{");
        for (int cnt = 0; cnt < DXL_CNT; cnt++)
        {
          DEBUG_SERIAL.print(motion[num][cnt]);
          DEBUG_SERIAL.print(", ");
        }
        DEBUG_SERIAL.print("1.0, 0.0}");
        if ((num + 1) != page)
        {
          DEBUG_SERIAL.println(",");
        }
      }
      DEBUG_SERIAL.println("\n};");
    }
  }
}

void split(String data, char separator, String* temp)
{
  int cnt = 0;
  int get_index = 0;

  String copy = data;
  
  while(true)
  {
    get_index = copy.indexOf(separator);

    if(-1 != get_index)
    {
      temp[cnt] = copy.substring(0, get_index);

      copy = copy.substring(get_index + 1);
    }
    else
    {
      temp[cnt] = copy.substring(0, copy.length());
      break;
    }
    ++cnt;
  }
}
