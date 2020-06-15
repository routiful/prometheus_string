#include <DynamixelShield.h>
#include <SoftwareSerial.h>

//#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)
//  #include <SoftwareSerial.h>
//  SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
//  #define DEBUG_SERIAL soft_serial
//#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_SAM_ZERO)
//  #define DEBUG_SERIAL SerialUSB    
//#else
//  #define DEBUG_SERIAL Serial
//#endif

#define FRONT_DXL 1
#define LEFT_DXL  2
#define ROOM_DXL  3

const float UP = 1.0; // CW
const float DOWN = -1.0; // CCW

const float DXL_ONE_ROTATION = 4096.0;
const float DXL_HALF_ROTATION = 2048.0;
const float DXL_ZERO_ROTATION = 0.0;

// Ultrasoinc
#define TRIG_PIN  13
#define ECHO_PIN  12

SoftwareSerial soft_serial(10, 11); // DYNAMIXELShield UART RX/TX
#define BLUETOOTH soft_serial
#define DEBUG_SERIAL Serial

DynamixelShield dxl_shield;
const float DXL_PROTOCOL_VERSION = 2.0;
int8_t dxl[3] = {FRONT_DXL, LEFT_DXL, ROOM_DXL};

class Ultrasonic
{
 public:
  Ultrasonic(){}
  ~Ultrasonic(){}

  init(int set_trig_pin, int set_echo_pin)
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
  DEBUG_SERIAL.begin(9600);
  BLUETOOTH.begin(9600);
//  while(!DEBUG_SERIAL);

  dxl_shield.begin(1000000);
  dxl_shield.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  dxl_shield.ping(FRONT_DXL);
  dxl_shield.ping(ROOM_DXL);
  dxl_shield.ping(LEFT_DXL);

  dxl_shield.torqueOff(FRONT_DXL);
  dxl_shield.torqueOff(ROOM_DXL);
  dxl_shield.torqueOff(LEFT_DXL);

  const int8_t EXTENDED_POSITION_CONTROL_MODE = 4;
  dxl_shield.writeControlTableItem(OPERATING_MODE, FRONT_DXL, EXTENDED_POSITION_CONTROL_MODE);
  dxl_shield.writeControlTableItem(OPERATING_MODE, ROOM_DXL, EXTENDED_POSITION_CONTROL_MODE);
  dxl_shield.writeControlTableItem(OPERATING_MODE, LEFT_DXL, EXTENDED_POSITION_CONTROL_MODE);

  const int8_t TIME_BASED_PROFILE = 4;
  dxl_shield.writeControlTableItem(DRIVE_MODE, FRONT_DXL, TIME_BASED_PROFILE);
  dxl_shield.writeControlTableItem(DRIVE_MODE, ROOM_DXL, TIME_BASED_PROFILE);
  dxl_shield.writeControlTableItem(DRIVE_MODE, LEFT_DXL, TIME_BASED_PROFILE);

  dxl_shield.writeControlTableItem(HOMING_OFFSET, FRONT_DXL, 0);
  dxl_shield.writeControlTableItem(HOMING_OFFSET, ROOM_DXL, 0);
  dxl_shield.writeControlTableItem(HOMING_OFFSET, LEFT_DXL, 0);

  dxl_shield.writeControlTableItem(HOMING_OFFSET, FRONT_DXL, -1 * dxl_shield.getPresentPosition(FRONT_DXL));
  dxl_shield.writeControlTableItem(HOMING_OFFSET, ROOM_DXL, -1 * dxl_shield.getPresentPosition(ROOM_DXL));
  dxl_shield.writeControlTableItem(HOMING_OFFSET, LEFT_DXL, -1 * dxl_shield.getPresentPosition(LEFT_DXL));
  
  dxl_shield.torqueOn(FRONT_DXL);
  dxl_shield.torqueOn(ROOM_DXL);
  dxl_shield.torqueOn(LEFT_DXL);

  ultrasonic.init(TRIG_PIN, ECHO_PIN);
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
  float dir = 0.0;
  if (present_height <= goal_height) 
  {
    dir = UP;
  }
  else 
  {
    dir = DOWN;
  }

      DEBUG_SERIAL.print(PULLEY_RADIUS);
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(PULLEY_BORDER_LENGTH);
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(HEIGHT_PER_ONE_DXL_UNIT);
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(dxl_shield.getPresentPosition(id, UNIT_RAW));
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(present_height);
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(goal_height);
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.println(dir);

  to_rotation(id, dir, goal_height / HEIGHT_PER_ONE_DXL_UNIT, move_time);
}

void loop() 
{
  static uint32_t tick = millis();
  if (Serial.available() > 0) 
  {
    if ((millis()-tick) >= 100)
    {     
      if (dxl_shield.readControlTableItem(MOVING, FRONT_DXL))
      {
        return;
      }
      else
      {
        String read_string = Serial.readStringUntil('\n');
        Serial.println(String(read_string));
        
        move(DXL_ID, read_string.toInt()); // miilis
      }
#if 0
      static int check = 0;
      DEBUG_SERIAL.print("moving: ");
      DEBUG_SERIAL.print(dxl_shield.getPresentPosition(FRONT_DXL));
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.print(dxl_shield.getPresentPosition(ROOM_DXL));
      DEBUG_SERIAL.print(" ");
      DEBUG_SERIAL.println(dxl_shield.getPresentPosition(LEFT_DXL));
      
      if (dxl_shield.readControlTableItem(MOVING, FRONT_DXL) == false &&
          dxl_shield.readControlTableItem(MOVING, ROOM_DXL) == false &&
          dxl_shield.readControlTableItem(MOVING, LEFT_DXL) == false)
      {
        if (check%2)
        {
          to_rotation(FRONT_DXL, UP, DXL_HALF_ROTATION, 4000);
          to_rotation(ROOM_DXL, UP, DXL_HALF_ROTATION, 4000);
          to_rotation(LEFT_DXL, UP, DXL_HALF_ROTATION, 4000);
        }
        else
        {
          to_rotation(FRONT_DXL, DOWN, DXL_HALF_ROTATION, 4000);
          to_rotation(ROOM_DXL, DOWN, DXL_HALF_ROTATION, 4000);
          to_rotation(LEFT_DXL, DOWN, DXL_HALF_ROTATION, 4000);
        }
        check++;
      }
  #endif
      tick = millis();
    }
  }
}
