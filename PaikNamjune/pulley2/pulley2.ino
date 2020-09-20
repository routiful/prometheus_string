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
  Dynamixel2Arduino dxl_shield(DXL_SERIAL, DXL_DIR_PIN);
#else
  #define DEBUG_SERIAL Serial
  #include <DynamixelShield.h>
  DynamixelShield dxl_shield;
#endif

#define DXL_CNT 15

const float UP = 1.0; // CW
const float DOWN = -1.0; // CCW

const float DXL_ONE_ROTATION = 4096.0;
const float DXL_HALF_ROTATION = 2048.0;
const float DXL_ZERO_ROTATION = 0.0;

const float DXL_PROTOCOL_VERSION = 2.0;

#define STRING_BUF_NUM 64
String cmd[STRING_BUF_NUM];

void dxl_setup(uint8_t id)
{
  dxl_shield.ping(id);

  dxl_shield.torqueOff(id);
  
  const int8_t EXTENDED_POSITION_CONTROL_MODE = 4;
  dxl_shield.writeControlTableItem(OPERATING_MODE, id, EXTENDED_POSITION_CONTROL_MODE);

  const int8_t TIME_BASED_PROFILE = 4;
  dxl_shield.writeControlTableItem(DRIVE_MODE, id, TIME_BASED_PROFILE);
  dxl_shield.writeControlTableItem(HOMING_OFFSET, id, 0);
  dxl_shield.writeControlTableItem(HOMING_OFFSET, id, -1 * dxl_shield.getPresentPosition(id));

  dxl_shield.torqueOn(id);
}

void setup() 
{
  DEBUG_SERIAL.begin(115200);
//  while(!DEBUG_SERIAL);

  dxl_shield.begin(1000000);
  dxl_shield.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  
  for (uint8_t cnt = 0, id = 1; cnt < DXL_CNT; cnt++, id++)
  {
    dxl_setup(id);
  }
}

void set_profile(uint8_t id, int32_t move_time = 2000)
{
  dxl_shield.writeControlTableItem(PROFILE_ACCELERATION, id, 0);
  dxl_shield.writeControlTableItem(PROFILE_VELOCITY, id, move_time); 
}

void to_rotation(uint8_t id, float dir, float dxl_unit, int32_t move_time = 2000)
{
  set_profile(id, move_time);
  dxl_shield.setGoalPosition(id, dir * dxl_unit);
}

void from_rotation(uint8_t id, float dir, float dxl_unit, int32_t move_time = 2000)
{
  set_profile(id, move_time);

  static float goal_position = 0.0;
  goal_position = dxl_shield.getPresentPosition(id) + (dir * dxl_unit);
  dxl_shield.setGoalPosition(id, goal_position);
}

void move(uint8_t id, float goal_height, int32_t move_time = 2000)
{
  if (goal_height < 0.0) 
  {
    return;
  }

  const float PULLEY_RADIUS = 30.0; // millis
  const float PULLEY_BORDER_LENGTH = 2 * PI * PULLEY_RADIUS; // 188 millis per one rotation
  const float HEIGHT_PER_ONE_DXL_UNIT = PULLEY_BORDER_LENGTH / DXL_ONE_ROTATION;
  
  float present_height = dxl_shield.getPresentPosition(id, UNIT_RAW) * HEIGHT_PER_ONE_DXL_UNIT;

//  DEBUG_SERIAL.print(PULLEY_RADIUS);
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.print(PULLEY_BORDER_LENGTH);
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.print(HEIGHT_PER_ONE_DXL_UNIT);
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.print(dxl_shield.getPresentPosition(id, UNIT_RAW));
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.print(present_height);
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.print(goal_height);
//  DEBUG_SERIAL.print(" ");
//  DEBUG_SERIAL.println(dir);

  to_rotation(id, UP, goal_height / HEIGHT_PER_ONE_DXL_UNIT, move_time);
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

void loop() 
{
  static uint32_t tick = millis();
  if (DEBUG_SERIAL.available() > 0) 
  {
      String read_string = DEBUG_SERIAL.readStringUntil('\n');
//      DEBUG_SERIAL.println(String(read_string));

      read_string.trim();
      split(read_string, ' ', cmd);
  }

  uint32_t move_time = 50; // milliseconds
  if ((millis()-tick) >= move_time)
  {
    for (uint8_t cnt = 0, id = 1; cnt < DXL_CNT; cnt++, id++)
    {
      move(id, cmd[cnt].toInt(), move_time); // miilis
    }
//    move(FIRST_DXL, cmd[0].toInt(), move_time); // miilis
//    move(SECOND_DXL, cmd[1].toInt(), move_time); // miilis 
//    move(THIRD_DXL, cmd[2].toInt(), move_time); // miilis 
    
    tick = millis();
  }
}
