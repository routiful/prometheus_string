/* Authors: Taehun Lim (Darby) */

#include <DynamixelWorkbench.h>

#if defined(__OPENCM904__)
  #define DEVICE_NAME "1" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
  #define DEVICE_NAME ""
#endif

#define BAUDRATE  1000000
#define DXL_CNT 10
uint8_t dxl_id[DXL_CNT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

DynamixelWorkbench dxl_wb;

int32_t goal_position[2] = {0, 512};
int32_t moving_speed[2] = {512, 512};

const uint8_t goal_position_handler = 0;
const uint8_t moving_speed_handler = 1;

const int analogInPin = 0;
int sonic_data;

int led_pin = 14;

#define OPENED 0
#define CLOSED 512

#define FAST 100
#define SLOW 30

#define FAST_DELAY 2000
#define SLOW_DELAY 7000

class Ultrasonic
{
 public:
  Ultrasonic(){}
  ~Ultrasonic(){}

  void init(int set_trig_pin, int set_echo_pin)
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

void setup()
{
  Serial.begin(57600);
//  while(!Serial); // Wait for Opening Serial Monitor

  pinMode(analogInPin, INPUT_ANALOG);
  pinMode(led_pin, OUTPUT);

  ultrasonic.init(0, 1);

  const char *log;
  bool result = false;

  uint16_t model_number = 0;
  result = dxl_wb.init(DEVICE_NAME, BAUDRATE, &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to init");
  }
  else
  {
    Serial.print("Succeeded to init : ");
    Serial.println(BAUDRATE);
  }

  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    result = dxl_wb.ping(dxl_id[cnt], &model_number, &log);
    if (result == false)
    {
      Serial.println(log);
      Serial.println("Failed to ping");
    }
    else
    {
      Serial.println("Succeeded to ping");
      Serial.print("id : ");
      Serial.print(dxl_id[cnt]);
      Serial.print(" model_number : ");
      Serial.println(model_number);
      digitalWrite(led_pin, HIGH);
    }

    result = dxl_wb.jointMode(dxl_id[cnt], 0, 0, &log);
    if (result == false)
    {
      Serial.println(log);
      Serial.println("Failed to change joint mode");
    }
    else
    {
      Serial.println("Succeed to change joint mode");
      digitalWrite(led_pin, HIGH);
    }
    delay(50);
  }

  randomSeed(analogRead(1));
}

void fast()
{
  move(FAST, OPENED, FAST_DELAY);
  move(FAST, CLOSED, FAST_DELAY);
}

void slow()
{
  move(SLOW, OPENED, SLOW_DELAY);
  move(SLOW, CLOSED, SLOW_DELAY);
}

void fast_slow()
{
  move(FAST, OPENED, FAST_DELAY);
  move(SLOW, CLOSED, SLOW_DELAY);
}

void slow_fast()
{
  move(SLOW, CLOSED, SLOW_DELAY);
  move(FAST, OPENED, FAST_DELAY);
}

void swing()
{
  for (int i = 0; i < 10; i++)
  {
    fast();
  }

  slow();
}

void stair_slow()
{
  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], SLOW, OPENED, SLOW_DELAY);
  }

  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], SLOW, CLOSED, SLOW_DELAY);
  }
}

void stair_fast()
{
  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], FAST, OPENED, FAST_DELAY);
  }

  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], FAST, CLOSED, FAST_DELAY);
  }
}

void loop()
{
  const int MOTION_NUM = 7;
  const float CONTROL_PERIOD = 0.050; // sec

  static uint32_t t = millis();
  if ((millis() - t) >= (CONTROL_PERIOD * 1000))
  {
    sonic_data = ultrasonic.get_distance();
    Serial.println(sonic_data);

    if (sonic_data < 100) // centimeter
    {
      long rand_num = random(1000);
      if (rand_num % MOTION_NUM == 0)
      {
        fast();
      }
      else if (rand_num % MOTION_NUM == 1)
      {
        slow();
      }
      else if (rand_num % MOTION_NUM == 2)
      {
        fast_slow();
      }
      else if (rand_num % MOTION_NUM == 3)
      {
        slow_fast();
      }
      else if (rand_num % MOTION_NUM == 4)
      {
        swing();
      }
      else if (rand_num % MOTION_NUM == 5)
      {
        stair_fast();
      }
      else if (rand_num % MOTION_NUM == 6)
      {
        stair_slow();
      }
    }

    t = millis();
  }
}

void set_position(int32_t pos)
{
  if (pos > 512) pos = 512;
  if (pos < 0) pos = 0;

  goal_position[0] = pos;
}

void set_speed(int32_t spd)
{
  if (spd > 1023) spd = 1023;
  if (spd < 0) spd = 0;

  moving_speed[0] = spd;
}

void move(int32_t spd, int32_t pos, uint32_t wait)
{
  speed(spd);

  set_position(pos);

  for (int id = 0; id <= DXL_CNT; id++)
  {
    dxl_wb.goalPosition(id, goal_position[0]);
  }

  delay(wait+50);
}

void move(int32_t id, int32_t spd, int32_t pos, uint32_t wait)
{
  speed(spd);

  set_position(pos);

  dxl_wb.goalPosition(id, goal_position[0]);

  delay(wait);
}

void speed(int32_t spd)
{
  set_speed(spd);

  for (int id = 0; id <= DXL_CNT; id++)
  {
    dxl_wb.goalVelocity(id, moving_speed[0]);
  }
}
