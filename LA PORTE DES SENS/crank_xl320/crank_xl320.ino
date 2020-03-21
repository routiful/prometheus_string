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

#define FAST 1023
#define SLOW 100

void setup() 
{
  Serial.begin(57600);
//  while(!Serial); // Wait for Opening Serial Monitor

  pinMode(analogInPin, INPUT_ANALOG);
  pinMode(led_pin, OUTPUT);

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
  move(FAST, OPENED, 1000);
  move(FAST, CLOSED, 1000);
}

void slow()
{
  move(SLOW, OPENED, 3000);
  move(SLOW, CLOSED, 3000);
}

void fast_slow()
{
  move(FAST, OPENED, 1000);
  move(SLOW, CLOSED, 3000);
}

void slow_fast()
{
  move(SLOW, OPENED, 1000);
  move(FAST, CLOSED, 3000);
}

void swing()
{
  for (int i = 0; i < 10; i++)
  {
    move(FAST, OPENED, 10);
    move(FAST, CLOSED, 10);
  }

  slow();
}

void stair_slow()
{
  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], SLOW, OPENED, 1000);
  }

  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], SLOW, CLOSED, 1000);
  }
}

void stair_fast()
{
  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], FAST, OPENED, 100);
  }

  for (int cnt = 0; cnt < DXL_CNT; cnt++)
  {
    move(dxl_id[cnt], FAST, CLOSED, 100);
  }
}

void loop() 
{ 
  const int MOTION_NUM = 7;
  const float CONTROL_PERIOD = 0.050; // sec
  
  static uint32_t t = millis();
  if ((millis() - t) >= (CONTROL_PERIOD * 1000))
  {
    sonic_data = analogRead(analogInPin) * 3;
  
    if (sonic_data < 1000)
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

  delay(wait);
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
