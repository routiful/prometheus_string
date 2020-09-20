/* Authors: Darby Lim */

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)
  #include <SoftwareSerial.h>
  SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
  #define DEBUG_SERIAL soft_serial
#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_SAM_ZERO)
  #define DEBUG_SERIAL SerialUSB
#elif defined(ARDUINO_OpenCR) // When using official ROBOTIS board with DXL circuit.
  // For OpenCR, there is a DXL Power Enable pin, so you must initialize and control it.
  // Reference link : https://github.com/ROBOTIS-GIT/OpenCR/blob/master/arduino/opencr_arduino/opencr/libraries/DynamixelSDK/src/dynamixel_sdk/port_handler_arduino.cpp#L78
  #define DXL_SERIAL   Serial3
  #define DEBUG_SERIAL Serial
  #include <Dynamixel2Arduino.h>
  const uint8_t DXL_DIR_PIN = 84; // OpenCR Board's DIR PIN. 
  Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
#else
  #define DEBUG_SERIAL Serial
  #include <DynamixelShield.h>
  DynamixelShield dxl;
#endif

// #define GET_MOTION
#define PLAY_MOTION

const int32_t DXL_BAUDRATE = 1000000;
const float DXL_PROTOCOL_VERSION = 1.0;

const uint8_t DXL_CNT = 4;
const uint8_t DXL_ID[DXL_CNT+DXL_CNT] = {1, 2, 3, 4, 5, 6, 7, 8};

const uint8_t MOVE_TIME_CNT = 1;
const uint8_t DELAY_TIME_CNT = 1;

int8_t page_cnt_ = -1;
uint32_t delay_t_ = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t DEFAULT_MOTION_PAGE = 20;
const float default_motion_[DEFAULT_MOTION_PAGE][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT] = {
  // deg ... move_time(sec)(From n-1 to n), delay_time(sec)(Fron n to n+1)
  {149.93, 149.06, 145.29, 147.03, 3.00, 2.00},
  {145.87, 80.62, 231.42, 148.19, 3.00, 2.00},
  {146.16, 80.62, 231.42, 240.00, 5.00, 2.00},
  {146.16, 80.62, 231.71, 60.00, 5.00, 2.00},
  {146.16, 80.62, 231.42, 240.00, 5.00, 2.00},
  {145.87, 80.04, 231.71, 60.00, 5.00, 2.00},
  {145.87, 80.33, 232.29, 148.77, 3.00, 1.00},
  {148.77, 147.32, 145.58, 148.77, 2.00, 1.00},
  {148.77, 223.59, 44.95, 149.35, 2.00, 1.00},
  {148.77, 227.65, 44.95, 240.00, 5.00, 2.00},
  {149.93, 227.94, 44.95, 60.00, 5.00, 2.00},
  {149.93, 228.52, 44.95, 240.00, 5.00, 2.00},
  {150.22, 227.94, 44.95, 60.00, 5.00, 2.00},
  {150.80, 228.23, 44.95, 152.83, 3.00, 1.00},
  {147.03, 147.03, 145.00, 151.38, 2.00, 1.00},
  {189.95, 179.51, 193.72, 221.27, 3.00, 1.00},
  {91.93, 67.86, 206.77, 59.16, 4.00, 3.00},
  {212.28, 225.62, 102.08, 229.39, 4.00, 3.00},
  {103.53, 68.44, 210.54, 74.53, 4.00, 3.00},
  {149.93, 149.06, 145.29, 147.03, 4.00, 3.00}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float CONTROL_PERIOD = 0.050; // sec

void setup() 
{
  DEBUG_SERIAL.begin(57600);
//  while(!DEBUG_SERIAL);

  dxl.begin(DXL_BAUDRATE);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  int id = 0;
  for (id = DXL_ID[0]; id <= DXL_ID[DXL_CNT-1]; id++)
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

void move(const float motion[][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT], uint8_t page_num)
{  
  const float DEGPERSEC_TO_RPM = 0.17f;
  const float DXL_AX12_RPM_UNIT = 0.111f;
  const uint32_t MOVE_TIME = DXL_CNT + MOVE_TIME_CNT - 1;
  const uint32_t DELAY_TIME = DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT - 1;
  
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
        page_cnt_ = -1;
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
    
    dxl.writeControlTableItem(MOVING_SPEED, id, (uint32_t)rpm_per_sec);
    dxl.setGoalPosition(id, motion[page_cnt_][num], UNIT_DEGREE);

    diff_deg = dxl.getPresentPosition(id + DXL_CNT, UNIT_DEGREE) - motion[page_cnt_][num];
    deg_per_sec = fabs(diff_deg) / motion[page_cnt_][MOVE_TIME];
    rpm_per_sec = deg_per_sec * DEGPERSEC_TO_RPM / DXL_AX12_RPM_UNIT;
    
    dxl.writeControlTableItem(MOVING_SPEED, id + DXL_CNT, (uint32_t)rpm_per_sec);
    dxl.setGoalPosition(id + DXL_CNT, motion[page_cnt_][num], UNIT_DEGREE);

//    DEBUG_SERIAL.print("id "); DEBUG_SERIAL.print(id);
//    DEBUG_SERIAL.print(" pos "); DEBUG_SERIAL.print(fabs(diff_deg));
//    DEBUG_SERIAL.print(" deg_per_sec "); DEBUG_SERIAL.print(deg_per_sec);
//    DEBUG_SERIAL.print(" rpm_per_sec "); DEBUG_SERIAL.print(rpm_per_sec);
//    DEBUG_SERIAL.print(" page_cnt "); DEBUG_SERIAL.print(page_cnt_);
  }
}

void loop()
{
#ifdef PLAY_MOTION
  static uint32_t t = millis();
  if ((millis() - t) >= (CONTROL_PERIOD * 1000))
  {
    move(default_motion_, DEFAULT_MOTION_PAGE);
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
  float motion[30][DXL_CNT + MOVE_TIME_CNT + DELAY_TIME_CNT];
  
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
