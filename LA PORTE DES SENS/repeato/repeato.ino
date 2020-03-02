/* Authors: Darby Lim */

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

const float CONTROL_PERIOD = 0.008; //sec

void setup() 
{
  DEBUG_SERIAL.begin(9600);

  dxl.begin(DXL_BAUDRATE);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int id = DXL_ID[0]; id <= DXL_ID[DXL_CNT-1]; id++)
  {
    dxl.ping(id);
    dxl.torqueOff(id);
    dxl.setOperatingMode(id, OP_POSITION);
    dxl.torqueOn(id);
  }
}

void move(uint32_t * goal_pos, float move_time)
{
  const float acc_time = 0.3, decel_time = 0.3;

  for (int id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
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

void dxl_position_controller(uint32_t * pre_pos, uint32_t * goal_pos, float acc_time, float dec_time, float total_time)
{
  // trapezoidal_time_profile

  float acceleration[DXL_CNT];
  float deceleration[DXL_CNT];
  float max_velocity[DXL_CNT];
  
  float accel_time[DXL_CNT];
  float const_time[DXL_CNT];
  float decel_time[DXL_CNT];
  
  float const_start_time[DXL_CNT];
  float decel_start_time[DXL_CNT];

  float velocity[DXL_CNT];
  float position[DXL_CNT];
  
  float move_time[DXL_CNT];
  uint32_t move_cnt = 0;
  
  for (int num = 0; num < DXL_CNT; num++)
  {
//    lookso_jr_dxl_present_rad_[id] = LooksoJrJoint.convertValue2Radian(pre_pos[id]);
//    lookso_jr_dxl_goal_rad_[id]   = goal_pos[id]*DEGREE2RADIAN;
    move_time[num]  = fabs(total_time);

    if ((fabs(acc_time) + fabs(dec_time)) <= move_time[num])
    {
      accel_time[num] = fabs(acc_time);
      decel_time[num] = fabs(dec_time);
      const_time[num] = move_time[num] - accel_time[num] - decel_time[num];
    }
    else
    {
      float time_gain = move_time[num] / (fabs(acc_time) + fabs(dec_time));
      accel_time[num] = time_gain*fabs(acc_time);
      decel_time[num] = time_gain*fabs(dec_time);
      const_time[num] = 0;
    }

    const_start_time[num] = accel_time[num];
    decel_start_time[num] = accel_time[num] + const_time[num];

    uint32_t pos_diff = goal_pos[num] - pre_pos[num];
    max_velocity[num] = 2 * pos_diff / (move_time[num] + const_time[num]);
    acceleration[num] = max_velocity[num] / accel_time[num];
    deceleration[num] = -max_velocity[num] / decel_time[num];
  }

  for (int num = 0; num < DXL_CNT; num++)
  {
    if (move_cnt * CONTROL_PERIOD < const_start_time[num])
    {
      velocity[num] = velocity[num] + (acceleration[num] * CONTROL_PERIOD);
      position[num] = position[num] + (velocity[num] * CONTROL_PERIOD);
//      lookso_jr_dxl_present_pos_[num] = LooksoJrJoint.convertRadian2Value(lookso_jr_dxl_present_rad_[num]);
    }
    else if ((move_cnt * CONTROL_PERIOD >= const_start_time[num]) && (move_cnt * CONTROL_PERIOD < decel_start_time[num]))
    {
      velocity[num] = max_velocity[num];
      position[num] = position[num] + (velocity[num] * CONTROL_PERIOD);
//      lookso_jr_dxl_present_pos_[num] = LooksoJrJoint.convertRadian2Value(lookso_jr_dxl_present_rad_[num]);
    }
    else if (move_cnt * CONTROL_PERIOD <= move_time[num])
    {
      velocity[num] = velocity[num] + (deceleration[num] * CONTROL_PERIOD);
      position[num] = position[num] + (velocity[num] * CONTROL_PERIOD);
//      lookso_jr_dxl_present_pos_[num] = LooksoJrJoint.convertRadian2Value(lookso_jr_dxl_present_rad_[num]);
    }
    else
    {
      break;
    }
    move_cnt++;
  }  
  
  for (int id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
  {
    dxl.setGoalPosition(id, position[num]);
  }
}

void loop()
{
#ifdef GET_MOTION
  const uint8_t STRING_BUF_NUM = 10
  String cmd[STRING_BUF_NUM];

  const uint8_t PAGE = 10;
  const uint8_t MOVE_TIME = 1;
  static uint32_t motion_[PAGE][DXL_CNT + MOVE_TIME];
  static uint8_t page = 0;
  
  if (DEBUG_SERIAL.available() > 0)
  {
    String str = DEBUG_SERIAL.readStringUntil('\n');
//    DEBUG_SERIAL.println("[CMD] : " + String(str));

    str.trim();
    split(str, ' ', cmd);
    
    if (cmd[0] == "g")
    {
      for (int id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
      {
        motion_[page][num] = dxl.getPresentPosition(id);
        DEBUG_SERIAL.print(dxl_present_position[num]);
        DEBUG_SERIAL.print(" | ");
      }
      motion_[page][MOVE_TIME - 1] = cmd[1];
      
      DEBUG_SERIAL.print("| ");
      DEBUG_SERIAL.println(cmd[1]);
      
      page++;
    }
    else if (cmd == 'e' || page >= SAVE_PAGE)
    {
      for (int num = 0; num < page; num++)
      {
        for (int cnt = 0; id < DXL_CNT; cnt++)
        {
          DEBUG_SERIAL.print("dxl_goal_position_[");
          DEBUG_SERIAL.print(cnt);
          DEBUG_SERIAL.print("] = ");
          DEBUG_SERIAL.print(motion_[num][cnt]);
          DEBUG_SERIAL.print(";");
        }
        DEBUG_SERIAL.println("");
        DEBUG_SERIAL.println("move(dxl_goal_position_, 1.0);");
      }
    }
  }
#endif

#ifdef PLAY_MOTION
  static uint32_t t = millis();
  if ((t-millis()) >= CONTROL_PERIOD * 1000)
  {
    motion();
    t = millis();
  }
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
