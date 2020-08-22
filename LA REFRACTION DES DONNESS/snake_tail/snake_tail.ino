#include <DynamixelShield.h>

/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Taehun Lim (Darby) */

#include <DynamixelWorkbench.h>

#define WAIT_FOR_BUTTON_PRESS 0
#define WAIT_SECOND 1
#define CHECK_BUTTON_RELEASED 2

#define SW_PIN 7

#define TRIG_PIN  9
#define ECHO_PIN  8

#define DEG2RAD 0.0174533f

#define DEVICE_NAME ""
#define BAUDRATE  1000000

#define DXL_ID_1  1
#define DXL_ID_2  2
#define DXL_ID_3  3
#define DXL_ID_4  4

#define GOAL_POSITION_SYNC_WRITE_HANDLER 0
#define PROFILE_VELOCITY_SYNC_WRITE_HANDLER 1

#define POSE_OFFSET 50

#define MOVE_TIME 5000 //ms

uint32_t tTime = 0;

DynamixelWorkbench dxl_wb;

uint8_t dxl_id[4] = {DXL_ID_1, DXL_ID_2, DXL_ID_3, DXL_ID_4};
const uint8_t dxl_cnt = 4;

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

void move(uint32_t move_time, float motor_1, float motor_2, float motor_3, float motor_4)
{
  const char *log;
  bool result = false;

  int32_t goal_position[4] = {0,0,0,0};
  int32_t profile_velocity[4] = {0, 0, 0, 0};

  profile_velocity[0] = move_time;
  profile_velocity[1] = move_time;
  profile_velocity[2] = move_time;
  profile_velocity[3] = move_time;
  
  result = dxl_wb.syncWrite(PROFILE_VELOCITY_SYNC_WRITE_HANDLER, &profile_velocity[0], &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to sync write profile velocity");
  }

  goal_position[0] = getValue(motor_1 * DEG2RAD);
  goal_position[1] = getValue(motor_2 * DEG2RAD);
  goal_position[2] = getValue(motor_3 * DEG2RAD);
  goal_position[3] = getValue(motor_4 * DEG2RAD);

  result = dxl_wb.syncWrite(GOAL_POSITION_SYNC_WRITE_HANDLER, &goal_position[0], &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to sync write position");
  }

  delay(move_time + 10);
}

int32_t getValue(float radian)
{
  int32_t value = 0;

  if (radian > 0)
  {
    value = (radian * (4096 - 2048) / 3.14) + 2048;
  }
  else if (radian < 0)
  {
    value = (radian * (0 - 2048) / -3.14) + 2048;
  }
  else
  {
    value = 2048;
  }

  return value;
}

bool getButtonPress(uint16_t time_to_press)
{
  bool result = false;
  static uint32_t t_time = 0;
  static uint8_t button_state = 0;

  switch (button_state)
  {
    case WAIT_FOR_BUTTON_PRESS:
      if (digitalRead(SW_PIN) == LOW)
      {
        t_time = millis();
        button_state = WAIT_SECOND;
      }
      break;

    case WAIT_SECOND:
      if ((millis()-t_time) >= time_to_press)
      {
        if (digitalRead(SW_PIN) == LOW)
        {
          button_state = CHECK_BUTTON_RELEASED;
          result = true;
        }
        else
        {
          button_state = WAIT_FOR_BUTTON_PRESS;
        }
      }
      break;

    case CHECK_BUTTON_RELEASED:
      if (digitalRead(SW_PIN) == HIGH)
        button_state = WAIT_FOR_BUTTON_PRESS;
      break;

    default :
      button_state = WAIT_FOR_BUTTON_PRESS;
      break;
  }

  return result;
}

void checkDXLError()
{
  const char *log;
  bool result = false;
  
  for (int cnt = 0; cnt < dxl_cnt; cnt++)
  {
    int32_t get_data = 0;
    result = dxl_wb.itemRead(dxl_id[cnt], "Hardware_Error_Status", &get_data, &log);
    if (result == false)
    {
      Serial.println(log);
      Serial.println("Failed to get hardware error status");

      result = dxl_wb.reboot(dxl_id[cnt], &log);
      if (result == false)
      {
        Serial.println(log);
        Serial.println("Failed to reboot");
      }
      else
      {
        Serial.println("Succeed to reboot");
      }

      result = dxl_wb.jointMode(dxl_id[cnt], 3000, 0, &log);
      if (result == false)
      {
        Serial.println(log);
        Serial.println("Failed to change joint mode");
      }
      else
      {
        Serial.println("Succeed to change joint mode");
      }
    }
    else
    {
      Serial.print("Succeed to get hardware error status(value : ");
      Serial.print(get_data);
      Serial.print(" ");
      Serial.print(dxl_id[cnt]);
      Serial.println(")");
    }
  }
}

void setup() 
{
  Serial.begin(57600);
//  while(!Serial); // If this line is activated, you need to open Serial Terminal.

  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(BDPIN_LED_USER_1, OUTPUT);

  ultrasonic.init(TRIG_PIN, ECHO_PIN);

  const char *log;
  bool result = false;

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
  
  for (int cnt = 0; cnt < dxl_cnt; cnt++)
  {
    uint16_t model_number = 0;
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
    }
    
    result = dxl_wb.jointMode(dxl_id[cnt], 3000, 0, &log);
    if (result == false)
    {
      Serial.println(log);
      Serial.println("Failed to change joint mode");
    }
    else
    {
      Serial.println("Succeed to change joint mode");
    }
  }

  result = dxl_wb.addSyncWriteHandler(dxl_id[0], "Goal_Position", &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to add goal position sync write handler");
  }

  result = dxl_wb.addSyncWriteHandler(dxl_id[0], "Profile_Velocity", &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to add profile velocity sync write handler");
  }
  
  move(3000, 0 + POSE_OFFSET, 0, 0, 0);
}

void loop() 
{  
  static uint8_t motion_cnt = 0;
  static bool flag = false;

  checkDXLError();

  if (flag == false)
  {
    if (getButtonPress(500) == true)
    {
      flag = true;
    }
    
    float distance = ultrasonic.get_distance();
    Serial.print("ultrasonic : ");
    Serial.println(distance);

    if (distance < 100.0)
    {
      flag = true;
    }
  }

  switch (motion_cnt)
  {
    case 0:
      if (flag == true)
      {
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);

        flag = false;
        motion_cnt = 1;
      }
     break;

    case 1:
      if (flag == true)
      {
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);
        move(8000, 60.0 + POSE_OFFSET, -40.0, 40.0, -40.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(2000, -40.0 + POSE_OFFSET, 20.0, -20.0, 60.0);
        move(2000, 40.0 + POSE_OFFSET, -20.0, 20.0, -60.0);
        move(8000, -60.0 + POSE_OFFSET, 40.0, -40.0, 40.0);

        flag = false;
        motion_cnt = 0;
      }
     break;

    default:
     break;
  }
}
