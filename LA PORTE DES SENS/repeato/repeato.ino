/* Authors: Darby Lim */
#include <MsTimer2.h>
#include <DynamixelShield.h>
DynamixelShield dxl;

#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial

// #define GET_MOTION
 #define PLAY_MOTION

const uint32_t DXL_BAUDRATE = 1000000;
const float DXL_PROTOCOL_VERSION = 2.0;

const uint8_t DXL_CNT = 4;
const uint8_t DXL_ID[DXL_CNT] = {11, 12, 13, 14};

uint32_t dxl_present_position_[DXL_CNT];
uint32_t dxl_goal_position_[DXL_CNT];

void timer_init()
{
  uint32_t ms = 8;
  MsTimer2::set(ms, run);
  MsTimer2::start();
}

void setup() 
{
  Serial.begin(9600);
  
  dxl.begin(DXL_BAUDRATE);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int id = DXL_ID[0]; id < DXL_CNT; id++)
  {
    dxl.ping(id);
    dxl.torqueOff(id);
    dxl.setOperatingMode(id, OP_POSITION);
    dxl.torqueOn(id);
  }

  timer_init();
}

void move(uint32_t * goal_pos, float move_time)
{
  const float acc_time = 0.3, decel_time = 0.3;

  for (int id = DXL_ID[0], num = 0; id < DXL_CNT; id++, num++)
  {
    dxl_present_position_[num] = dxl.getPresentPosition(id);
  }
  dxl_position_controller(dxl_present_position_, goal_pos, acc_time, decel_time, move_time);
}

void motion()
{
  dxl_goal_position_[0] = 2048;
  dxl_goal_position_[1] = 2048;
  dxl_goal_position_[2] = 2048;
  dxl_goal_position_[3] = 2048;
  move(dxl_goal_position_, 1.0);

  dxl_goal_position_[0] = 0;
  dxl_goal_position_[1] = 0;
  dxl_goal_position_[2] = 0;
  dxl_goal_position_[3] = 0;
  move(dxl_goal_position_, 2.0);
}

void dxl_position_controller(uint32_t * pre_pos, uint32_t * goal_pos, float acc_time, float decel_time, float move_time)
{
  // trapezoidal_time_profile
  for (int id = DXL_ID[0], num = 0; id < DXL_CNT; id++, num++)
  {
//    lookso_jr_dxl_present_rad_[id] = LooksoJrJoint.convertValue2Radian(pre_pos[id]);
//    lookso_jr_dxl_goal_rad_[id]   = goal_pos[id]*DEGREE2RADIAN;
//    move_time_[id]  = fabs(total_time);
//
//    if ((fabs(acc_time) + fabs(decel_time)) <= move_time_[id])
//    {
//      accel_time_[id] = fabs(acc_time);
//      decel_time_[id] = fabs(decel_time);
//      const_time_[id] = move_time_[id] - accel_time_[id] - decel_time_[id];
//    }
//    else
//    {
//      float time_gain = move_time_[id] / (fabs(acc_time) + fabs(decel_time));
//      accel_time_[id] = time_gain*fabs(acc_time);
//      decel_time_[id] = time_gain*fabs(decel_time);
//      const_time_[id] = 0;
//    }
//
//    const_start_time_[id] = accel_time_[id];
//    decel_start_time_[id] = accel_time_[id] + const_time_[id];
//
//    float pos_diff = lookso_jr_dxl_goal_rad_[id] - lookso_jr_dxl_present_rad_[id];
//    max_velocity_[id] = 2*pos_diff / (move_time_[id] + const_time_[id]);
//    acceleration_[id] = max_velocity_[id] / accel_time_[id];
//    deceleration_[id] = -max_velocity_[id] / decel_time_[id];
//  }
}

void loop() {}
void run()
{
#ifdef GET_MOTION
  const uint8_t STRING_BUF_NUM = 10
  String cmd[STRING_BUF_NUM];

  const uint8_t PAGE = 10;
  const uint8_t MOVE_TIME = 1;
  static uint32_t motion_[PAGE][DXL_CNT + MOVE_TIME];
  static uint8_t page = 0;
  
  if (Serial.available() > 0)
  {
    String str = Serial.readStringUntil('\n');
//    Serial.println("[CMD] : " + String(str));

    str.trim();
    split(str, ' ', cmd);
    
    if (cmd[0] == "g")
    {
      for (int id = DXL_ID[0], num = 0; id < DXL_CNT; id++, num++)
      {
        motion_[page][num] = dxl.getPresentPosition(id);
        Serial.print(dxl_present_position[num]);
        Serial.print(" | ");
      }
      motion_[page][MOVE_TIME - 1] = cmd[1];
      
      Serial.print("| ");
      Serial.println(cmd[1]);
      
      page++;
    }
    else if (cmd == 'e' || page >= SAVE_PAGE)
    {
      for (int num = 0; num < page; num++)
      {
        for (int cnt = 0; id < DXL_CNT; cnt++)
        {
          Serial.print("dxl_goal_position_[");
          Serial.print(cnt);
          Serial.print("] = ");
          Serial.print(motion_[num][cnt]);
          Serial.print(";");
        }
        Serial.println("");
        Serial.println("move(dxl_goal_position_, 1.0);");
      }
    }
  }
#endif

#ifdef PLAY_MOTION
  motion();
#endif
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
