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

const int analogInPin = 0;
int sonic_data;

DynamixelWorkbench dxl_wb; 

int32_t goal_position[2] = {0, 512};
int32_t moving_speed[2] = {512, 512};

const uint8_t goal_position_handler = 0;
const uint8_t moving_speed_handler = 1;

#define DOWN 512
#define UP 0

void setup() 
{
  Serial.begin(57600);
//  while(!Serial); // Wait for Opening Serial Monitor

  pinMode(analogInPin, INPUT_ANALOG);

  const char *log;
  bool result = false;

  uint16_t model_number = 0;
  uint8_t dxl_id[DXL_CNT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

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

  for (int cnt = 0; cnt <= DXL_CNT; cnt++)
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
    }
  } 

  randomSeed(analogRead(1));
}

void fast()
{
  speed(1023);
  move(1000, DOWN);

  speed(1023);
  move(1000, UP);
}

void slow()
{
  speed(100);
  move(3000, DOWN);

  speed(100);
  move(3000, UP);
}

void fast_slow()
{
  speed(1023);
  move(1000, DOWN);

  speed(100);
  move(3000, UP);
}

void swing()
{
  for (int i = 0; i < 10; i++)
  {
    speed(1023);
    move(1, DOWN);
  
    speed(1023);
    move(1, UP);
  }

  speed(100);
  move(3000, DOWN);
}

void loop() 
{  
  sonic_data = analogRead(analogInPin) * 3;

  if (sonic_data < 1000)
  {
    long rand_num = random(1000);
    if (rand_num % 4 == 0)
    {
      fast();
    }
    else if (rand_num % 4 == 1)
    {
      slow();
    }
    else if (rand_num % 4 == 2)
    {
      fast_slow();
    }
    else if (rand_num % 4 == 3)
    {
      swing();
    }
  }
}

void set_position(int32_t a)
{
  if (a > 512) a = 512;
  if (a < 0) a = 0;
  
  goal_position[0] = a;
}

void set_speed(int32_t a)
{
  if (a > 1023) a = 1023;
  if (a < 0) a = 0;
  
  moving_speed[0] = a;
}

void move(uint32_t move_time, int32_t a)
{
  set_position(a);

  for (int id = 0; id <= DXL_CNT; id++)
  {
    dxl_wb.goalPosition(id, goal_position[0]);
  }

  delay(move_time);
}

void speed(int32_t a)
{
  set_speed(a);
  
  for (int id = 0; id <= DXL_CNT; id++)
  {
    dxl_wb.goalVelocity(id, moving_speed[0]);
  }
}
