/* Authors: Darby Lim */

#include <DynamixelWorkbench.h>

#if defined(__OPENCM904__)
  #define DEVICE_NAME "3" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
  #define DEVICE_NAME ""
#endif   

#define BAUDRATE 1000000

#define DXL_CNT 2
const uint8_t dxl_id[DXL_CNT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
int32_t goal_position[DXL_CNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const float UP = 1.0; // CW
const float DOWN = -1.0; // CCW

const float DXL_ONE_ROTATION = 4096.0;
const float DXL_HALF_ROTATION = 2048.0;
const float DXL_ZERO_ROTATION = 0.0;

#define STRING_BUF_NUM 64
String cmd[STRING_BUF_NUM];

const uint32_t MOVE_TIME = 100; // milliseconds

DynamixelWorkbench dxl_wb;

void dxl_setup(uint8_t id)
{
  const char *log;
  bool result = false;
  
  uint16_t model_number = 0;
  result = dxl_wb.ping(id, &model_number, &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to ping");
  }
  else
  {
    Serial.println("Succeeded to ping");
    Serial.print("id : ");
    Serial.print(id);
    Serial.print(" model_number : ");
    Serial.println(model_number);
  }

  dxl_wb.itemWrite(id, "Homing_Offset", 0);
  
  int32_t present_position = 0;
  dxl_wb.itemRead(id, "Present_Position", &present_position, &log);
  
  dxl_wb.itemWrite(id, "Homing_Offset", -1 * present_position);

  int32_t homing_offset = 0;
  result = dxl_wb.itemRead(id, "Homing_Offset", &homing_offset, &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to get homing offset");
  }
  else
  {
    Serial.print("Succeeded to homing offset ");
    Serial.println(homing_offset);
  }

  dxl_wb.itemWrite(id, "Profile_Velocity", MOVE_TIME);
  dxl_wb.itemWrite(id, "Profile_Acceleration", 0);

  dxl_wb.torqueOn(id);
}

void setup() 
{
  Serial.begin(115200);
//  while(!Serial);

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

  for (uint8_t cnt = 0, id = dxl_id[0]; cnt < DXL_CNT; cnt++, id++)
  {
    dxl_setup(id);
  }

  result = dxl_wb.addSyncWriteHandler(dxl_id[0], "Goal_Position", &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to add sync write handler");
  }
}

void move(String * goal_height)
{
  if (goal_height == NULL) 
  {
    return;
  }

  const float PULLEY_RADIUS = 30.0; // millis
  const float PULLEY_BORDER_LENGTH = 2 * PI * PULLEY_RADIUS; // 188 millis per one rotation
  const float HEIGHT_PER_ONE_DXL_UNIT = PULLEY_BORDER_LENGTH / DXL_ONE_ROTATION;

  for (uint8_t cnt = 0; cnt < DXL_CNT; cnt++)
  {
    goal_position[cnt] = goal_height[cnt].toInt() / HEIGHT_PER_ONE_DXL_UNIT;
  }

  const char *log;
  bool result = false;

  const uint8_t handler_index = 0;
  result = dxl_wb.syncWrite(handler_index, &goal_position[0], &log);
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to sync write position");
  }
}

void split(String data, char separator, String * temp)
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
  if (Serial.available() > 0) 
  {
      String read_string = Serial.readStringUntil('\n');
      Serial.println(String(read_string));

      read_string.trim();
      split(read_string, ' ', cmd);
  }

  if ((millis()-tick) >= MOVE_TIME)
  {
    move(cmd);
    tick = millis();
  }
}
