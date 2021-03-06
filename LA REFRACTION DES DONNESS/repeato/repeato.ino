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

#define ULTRA_TRIGGER 150.0

const int32_t DXL_BAUDRATE = 1000000;
const float DXL_PROTOCOL_VERSION = 1.0;

const uint8_t DXL_CNT = 4;
const uint8_t DXL_ID[DXL_CNT] = {1, 2, 3, 4};

const uint8_t MOVE_TIME_CNT = 1;
const uint8_t DELAY_TIME_CNT = 1;

int8_t page_cnt_ = -1;
uint32_t delay_t_ = 0;

bool trigger_ = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t DEFAULT_MOTION_PAGE = 16;
const float default_motion_[DEFAULT_MOTION_PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {
  // deg ... move_time(sec)(From n-1 to n), delay_time(sec)(Fron n to n+1)
  {213.44, 106.43, 103.82, 149.06, 2.00, 1.00},
  {242.15, 97.73, 93.38, 181.25, 2.00, 1.00},
  {246.21, 91.93, 84.10, 127.31, 3.00, 1.00},
  {215.47, 87.00, 128.18, 137.17, 2.00, 0.00},
  {192.27, 84.68, 111.36, 146.16, 3.00, 1.00},
  {91.35, 189.37, 161.24, 150.22, 2.00, 1.00},
  {95.70, 247.95, 81.49, 137.75, 2.00, 1.00},
  {104.98, 246.79, 88.45, 182.41, 3.00, 0.00},
  {224.46, 116.00, 91.06, 157.76, 2.00, 1.00},
  {244.47, 116.87, 92.51, 156.02, 3.00, 0.00},
  {205.61, 112.23, 92.51, 156.02, 2.00, 0.00},
  {221.27, 111.36, 92.51, 154.28, 3.00, 1.00},
  {208.22, 108.75, 92.22, 140.65, 2.00, 0.00},
  {227.07, 106.43, 89.32, 162.69, 3.00, 1.00},
  {218.37, 115.13, 87.87, 167.04, 2.00, 1.00},
  {209.09, 129.05, 88.45, 159.50, 3.00, 1.00}
};

const uint8_t REACT_MOTION_PAGE = 11;
const float react_motion_[REACT_MOTION_PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {
  // deg ... move_time(sec)(From n-1 to n), delay_time(sec)(Fron n to n+1)
  {188.22, 50.75, 92.22, 140.65, 0.00, 0.00},
  {188.22, 50.75, 92.22, 140.65, 0.00, 1.00},
  {228.22, 50.75, 92.22, 140.65, 2.00, 0.00},
  {188.22, 50.75, 92.22, 140.65, 2.00, 0.00},
  {228.22, 50.75, 92.22, 140.65, 2.00, 0.00},
  {188.22, 50.75, 92.22, 140.65, 2.00, 0.00},
  {213.44, 50.75, 92.22, 149.06, 2.00, 1.00},
  {242.15, 50.75, 92.22, 181.25, 2.00, 1.00},
  {246.21, 50.75, 92.22, 127.31, 3.00, 1.00},
  {215.47, 50.75, 92.22, 137.17, 2.00, 0.00},
  {192.27, 50.75, 92.22, 146.16, 3.00, 1.00}
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
//  while(!DEBUG_SERIAL);

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
  ultrasonic.init(12, 13);

  randomSeed(analogRead(0));
  page_cnt_ = random(0, DEFAULT_MOTION_PAGE-1);
  DEBUG_SERIAL.println("Ready to Start");
}

void move(float motion[][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT], uint8_t page_num)
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
    DEBUG_SERIAL.println(" MOVE");
    delay_t = millis();
    return;
  }
  else
  {
    if ((millis()-delay_t) >= (motion[page_cnt_][DELAY_TIME] * 1000))
    {
      DEBUG_SERIAL.println(" NEXT");
      page_cnt_++;
      if (page_cnt_ >= page_num)
      {
        trigger_ = false;
        page_cnt_ = random(0, DEFAULT_MOTION_PAGE-1);

        return;
      }
    }
    else
    {
      DEBUG_SERIAL.println(" WAIT");
      return;
    }
  }
  
  for (uint8_t id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
  {
    diff_deg = dxl.getPresentPosition(id, UNIT_DEGREE) - motion[page_cnt_][num];
    deg_per_sec = fabs(diff_deg) / motion[page_cnt_][MOVE_TIME];
    rpm_per_sec = deg_per_sec * DEGPERSEC_TO_RPM / DXL_AX12_RPM_UNIT;

//    DEBUG_SERIAL.print("id "); DEBUG_SERIAL.print(id);
//    DEBUG_SERIAL.print(" pos "); DEBUG_SERIAL.print(fabs(diff_deg));
//    DEBUG_SERIAL.print(" deg_per_sec "); DEBUG_SERIAL.print(deg_per_sec);
//    DEBUG_SERIAL.print(" rpm_per_sec "); DEBUG_SERIAL.print(rpm_per_sec);
//    DEBUG_SERIAL.print(" page_cnt "); DEBUG_SERIAL.print(page_cnt_);
    
    dxl.writeControlTableItem(MOVING_SPEED, id, (uint32_t)rpm_per_sec);
    dxl.setGoalPosition(id, motion[page_cnt_][num], UNIT_DEGREE);
  }
}

void loop()
{
#ifdef PLAY_MOTION
  static uint32_t t = millis();
  if ((millis() - t) >= (CONTROL_PERIOD * 1000))
  {
//    if (page_cnt_%3 == 0)
//    {
//      rgb_led.on(255, 0, 0);
//    }
//    else if (page_cnt_%3 == 1)
//    {
//      rgb_led.on(0, 255, 0);
//    }
//    else if (page_cnt_%3 == 2)
//    {
//      rgb_led.on(0, 0, 255);
//    }

    DEBUG_SERIAL.print("Page ");
    DEBUG_SERIAL.print(page_cnt_);

    DEBUG_SERIAL.print(" Trigger ");
    DEBUG_SERIAL.print(trigger_);
    
    DEBUG_SERIAL.print(" Ultra ");
    DEBUG_SERIAL.print(ultrasonic.get_distance());

    if (trigger_ == true)
    {
      move(react_motion_, REACT_MOTION_PAGE);
      rgb_led.on(0,255,0);
    }
    else
    { 
      if (ultrasonic.get_distance() <= ULTRA_TRIGGER)
      {
        trigger_ = true;
        page_cnt_ = 0;
      }
      else
      {
        move(default_motion_, DEFAULT_MOTION_PAGE);
        rgb_led.on(0, 0, 0);
      }
    }
    
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
    String read_string = DEBUG_SERIAL.readStringUntil('\n');
    read_string.trim();
    split(read_string, ' ', cmd);

    if (cmd[0] == "h" || cmd[0] == "help" || cmd[0] == "?")
    {
      DEBUG_SERIAL.println("Press 'g' to save positions of Dynamixels");
      DEBUG_SERIAL.println("Press 'd' to delete last positions of Dynamixels");
      DEBUG_SERIAL.println("Press 'e' to get a code that has a motion array");
    }
    else if (cmd[0] == "g")
    {
      DEBUG_SERIAL.print("Page ");
      DEBUG_SERIAL.print(page);
      
      for (int id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
      {
        motion[page][num] = dxl.getPresentPosition(id, UNIT_DEGREE);
        DEBUG_SERIAL.print(" | ");
        DEBUG_SERIAL.print(motion[page][num]);
      }
      motion[page][DXL_CNT + MOVE_TIME_CNT - 1] = cmd[1].toFloat();
      motion[page][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1] = cmd[2].toFloat();
      
      DEBUG_SERIAL.print(" move_time ");
      DEBUG_SERIAL.print(motion[page][DXL_CNT + MOVE_TIME_CNT - 1]);
      DEBUG_SERIAL.print(" delay_time  ");
      DEBUG_SERIAL.print(motion[page][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1]);
        
      page++;
      DEBUG_SERIAL.print("\n");
    }
    else if (cmd[0] == "d")
    {
      page--;
    }
    else if (cmd[0] == "e" || page >= 250)
    { 
      DEBUG_SERIAL.print("const uint8_t PAGE = ");
      DEBUG_SERIAL.print(page);
      DEBUG_SERIAL.println(";");

      DEBUG_SERIAL.println("const float motion_[PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {");
      DEBUG_SERIAL.println("\t// deg ... move_time(sec)(From n-1 to n), delay_time(sec)(Fron n to n+1)");
      
      for (int num = 0; num < page; num++)
      {
        DEBUG_SERIAL.print("\t{");
        for (int cnt = 0; cnt < DXL_CNT; cnt++)
        {
          DEBUG_SERIAL.print(motion[num][cnt]);
          DEBUG_SERIAL.print(", ");
        }
        DEBUG_SERIAL.print(motion[num][DXL_CNT + MOVE_TIME_CNT - 1]);
        DEBUG_SERIAL.print(", ");
        DEBUG_SERIAL.print(motion[num][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1]);
        DEBUG_SERIAL.print("}");
        
//        DEBUG_SERIAL.print("1.0, 0.0}");
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
