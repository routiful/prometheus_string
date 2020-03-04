/* Authors: Darby Lim */

#include <DynamixelShield.h>
DynamixelShield dxl;

#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial

// #define GET_MOTION
 #define PLAY_MOTION

const int32_t DXL_BAUDRATE = 1000000;
const float DXL_PROTOCOL_VERSION = 1.0;

const uint8_t DXL_CNT = 2;
const uint8_t DXL_ID[DXL_CNT] = {1, 2};

const uint8_t MOVE_TIME_CNT = 1;
const uint8_t DELAY_TIME_CNT = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t PAGE = 2;
const float motion_[PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {
  // deg ... move_time(sec), delay_time(sec)
  {0.0, 0.0, 2.0, 1.0},
  {150.0, 295.0, 2.0, 3.0}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float CONTROL_PERIOD = 0.050; // sec

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

  DEBUG_SERIAL.println("Ready to Start");
}

void move()
{
  const float DEGPERSEC_TO_RPM = 0.17f;
  const float DXL_AX12_RPM_UNIT = 0.111f;
  const uint8_t MOVE_TIME = DXL_CNT + MOVE_TIME_CNT - 1;
  const uint8_t DELAY_TIME = DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1;
  
  static uint8_t page_cnt = 0;
  
  float diff_deg;
  float deg_per_sec;
  float rpm_per_sec;

//  for (uint8_t id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
//  {
    if (dxl.readControlTableItem(MOVING, DXL_ID[0]) == 1)
    {
      return;
    }
    else
    {
      delay(motion_[page_cnt][DELAY_TIME] * 1000);
    }
//  }
  
  for (uint8_t id = DXL_ID[0], num = 0; id <= DXL_ID[DXL_CNT-1]; id++, num++)
  {
    diff_deg = dxl.getPresentPosition(id, UNIT_DEGREE) - motion_[page_cnt][num];
    deg_per_sec = fabs(diff_deg) / motion_[page_cnt][MOVE_TIME];
    rpm_per_sec = deg_per_sec * DEGPERSEC_TO_RPM / DXL_AX12_RPM_UNIT;

//    DEBUG_SERIAL.print("id "); DEBUG_SERIAL.print(id);
//    DEBUG_SERIAL.print(" pos "); DEBUG_SERIAL.print(fabs(diff_deg));
//    DEBUG_SERIAL.print(" deg_per_sec "); DEBUG_SERIAL.print(deg_per_sec);
//    DEBUG_SERIAL.print(" rpm_per_sec "); DEBUG_SERIAL.print(rpm_per_sec);
//    DEBUG_SERIAL.print(" page_cnt "); DEBUG_SERIAL.print(page_cnt);
    
    dxl.writeControlTableItem(MOVING_SPEED, id, (uint32_t)rpm_per_sec);
    dxl.setGoalPosition(id, motion_[page_cnt][num], UNIT_DEGREE);
  }

  
  page_cnt++;
  if (page_cnt >= PAGE)
  {
    page_cnt = 0;
  }
}

void loop()
{
#ifdef GET_MOTION
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
#endif

#ifdef PLAY_MOTION
  static uint32_t t = millis();
  if ((t-millis()) >= (CONTROL_PERIOD * 1000))
  {
    move();
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
