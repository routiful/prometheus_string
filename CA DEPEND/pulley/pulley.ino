#include <DynamixelShield.h>
#include <SoftwareSerial.h>

#define FRONT_DXL 1
#define ROOM_DXL  2

#define UP  -1 // CW
#define DOWN 1 // CCW
#define STOP 0

#define MOTOR_SPEED 200         // 모터 속도
#define DIFF_OFFSET 100  // 현재 엔코더값과 멈춰야 하는 값과의 차
#define PRIFILE_ACC 1000 // 엔코더가 DIFF_OFFSET을 넘어 갔을 때 멈춰야하는 시간, 클수록 짧음

#define ONE_ROTATION 4096
#define HALF_ROTATION 2048

#define TIME_OUT 5000

// Ultrasoinc
#define TRIG_PIN  13
#define ECHO_PIN  12

float ultra_dist = 0;
float ultra_smooth_dist = 0;

DynamixelShield dxl;

SoftwareSerial music_Serial(10, 11);

typedef enum 
{
  WAIT_FLAG = 0,
  FRONT_DXL_UP,
  ROOM_DXL_UP,
  FRONT_DXL_DOWN,
  ROOM_DXL_DOWN,
  BOTH_DXL_UP,
  BOTH_DXL_DOWN
}State;

State state;

int32_t front_dxl_des_pos = 0;
int32_t room_dxl_des_pos = 0;

void setInitAngle(uint8_t id, int32_t angle)
{
  dxl.torqueOff(id);
  dxl.setJointMode(id);
  dxl.torqueOn(id);
  dxl.setGoalAngle(id, angle);

  delay(1000);
}

void updateDesiredPosition(uint8_t which_dxl, int32_t dir, int32_t offset)
{
  if (which_dxl == FRONT_DXL)
  {
    front_dxl_des_pos = front_dxl_des_pos + (dir * offset);
  }
  else if (which_dxl == ROOM_DXL)
  {
    room_dxl_des_pos = room_dxl_des_pos + (dir * offset);
  }
}

void stop(uint8_t id, int32_t timeout = 0)
{
  dxl.setGoalSpeed(id, STOP);

  delay(timeout);
}             

void move(uint8_t id, int32_t dir, int32_t speed)
{
  dxl.setGoalSpeed(id, dir * speed);
}

int32_t diff(uint8_t which_dxl)
{
  if (which_dxl == FRONT_DXL)
  {
    return abs(dxl.getCurPosition(FRONT_DXL) - front_dxl_des_pos);
  }
  else if (which_dxl == ROOM_DXL)
  {
    return abs(dxl.getCurPosition(ROOM_DXL) - room_dxl_des_pos);
  }

  return 0;
}

void musicStart()
{
  music_Serial.println("A");
}

void musicStop()
{
  music_Serial.println("B");
}

float getDistance()
{
  float duration, distance;
  digitalWrite(TRIG_PIN, HIGH);
  delay(10);
  digitalWrite(TRIG_PIN, LOW);
 
  // echoPin 이 HIGH를 유지한 시간을 저장 한다.
  duration = pulseIn(ECHO_PIN, HIGH);
  // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
  // 340은 초당 초음파(소리)의 속도, 10000은 밀리세컨드를 세컨드로, 왕복거리이므로 2로 나눠준다.
  distance = ((float)(340 * duration) / 10000) / 2;

  return distance;
}

float averageFilter(const float dist)
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

void dxlDebugMsg()
{
  Serial.print("desired_position : ");
  Serial.print(front_dxl_des_pos);
  Serial.print("  ");
  Serial.print(room_dxl_des_pos);
  Serial.print(" present_position : ");
  Serial.print(dxl.getCurPosition(FRONT_DXL));
  Serial.print("  ");
  Serial.print(dxl.getCurPosition(ROOM_DXL));
  Serial.print(" diff_position : ");
  Serial.print(abs(dxl.getCurPosition(FRONT_DXL) - front_dxl_des_pos));
  Serial.print("  ");
  Serial.println(abs(dxl.getCurPosition(ROOM_DXL) - room_dxl_des_pos));
}

void ultraDebugMsg()
{
  Serial.print(ultra_dist);
  Serial.print("cm  ");
  Serial.print(ultra_smooth_dist);
  Serial.println("cm");
}

void setup() 
{
  Serial.begin(9600);
  music_Serial.begin(9600);
//  while(!Serial);

  // 다이나믹셀
  dxl.begin(1000000, DXL_PACKET_VER_2_0);
  delay(1000);
  
  dxl.ping();
  
  dxl.torqueOff(DXL_ALL_ID);
  dxl.setWheelMode(FRONT_DXL);
  dxl.setWheelMode(ROOM_DXL);
  dxl.torqueOn(DXL_ALL_ID);     

  int32_t profile_acc = PRIFILE_ACC;
  
  dxl.write(FRONT_DXL, 108, (uint8_t *)&profile_acc, 4, 100);
  dxl.write(ROOM_DXL, 108, (uint8_t *)&profile_acc, 4, 100);
//bool DynamixelShield::write(uint8_t id, uint16_t addr, uint8_t *p_data, uint16_t length, uint32_t timeout)

  front_dxl_des_pos = dxl.getCurPosition(FRONT_DXL);
  room_dxl_des_pos = dxl.getCurPosition(ROOM_DXL);

  updateDesiredPosition(FRONT_DXL, UP, ONE_ROTATION);

  // 초음파 센서 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() 
{   
  if (dxl.getDxlCount() > 0)
  {
//    dxlDebugMsg();
    
    switch(state)
    {
      case WAIT_FLAG:
        ultra_dist = getDistance();
        ultra_smooth_dist = averageFilter(ultra_dist);  
  
//        ultraDebugMsg()

        if (ultra_smooth_dist < 100.0)
        {
          state = FRONT_DXL_UP;
        }
       break;
       
      case FRONT_DXL_UP:
        if (diff(FRONT_DXL) < DIFF_OFFSET)
        {
          musicStop();
          stop(FRONT_DXL, TIME_OUT);
          
          
          updateDesiredPosition(ROOM_DXL, UP, ONE_ROTATION);
          state = ROOM_DXL_UP;          
        }
        else
        {
          move(FRONT_DXL, UP, MOTOR_SPEED);
          musicStart();
        }
        break;

      case ROOM_DXL_UP:
        if (diff(ROOM_DXL) < DIFF_OFFSET)
        {
          stop(ROOM_DXL, TIME_OUT);
          musicStop();
          
          updateDesiredPosition(FRONT_DXL, DOWN, ONE_ROTATION);
          state = FRONT_DXL_DOWN;          
        }
        else
        {
          move(ROOM_DXL, UP, MOTOR_SPEED);
          musicStart();
        }
        break;
        
      case FRONT_DXL_DOWN:
        if (diff(FRONT_DXL) < DIFF_OFFSET)
        {
          stop(FRONT_DXL, TIME_OUT);
          musicStop();
          
          updateDesiredPosition(ROOM_DXL, DOWN, ONE_ROTATION);
          state = ROOM_DXL_DOWN;          
        }
        else
        {
          move(FRONT_DXL, DOWN, MOTOR_SPEED/2);
          musicStart();
        }
        break;

      case ROOM_DXL_DOWN:
        if (diff(ROOM_DXL) < DIFF_OFFSET)
        {
          stop(ROOM_DXL, TIME_OUT);
          musicStop();
          
          updateDesiredPosition(FRONT_DXL, UP, ONE_ROTATION);
          updateDesiredPosition(ROOM_DXL, UP, ONE_ROTATION);
          state = BOTH_DXL_UP;          
        }
        else
        {
          move(ROOM_DXL, DOWN, MOTOR_SPEED/2);
          musicStart();
        }
        break;

      case BOTH_DXL_UP:        
        if (diff(FRONT_DXL) < DIFF_OFFSET)
        {
          stop(ROOM_DXL, 0);
          stop(FRONT_DXL, 0);
          musicStop();
          
          updateDesiredPosition(FRONT_DXL, DOWN, ONE_ROTATION);
          updateDesiredPosition(ROOM_DXL, DOWN, ONE_ROTATION);
          
          delay(TIME_OUT);
          
          state = BOTH_DXL_DOWN;
        }
        else
        {
          move(FRONT_DXL, UP, MOTOR_SPEED);
          move(ROOM_DXL, UP, MOTOR_SPEED);
          musicStart();
        }
        break;

      case BOTH_DXL_DOWN:
        if (diff(FRONT_DXL) < DIFF_OFFSET)
        {
          stop(ROOM_DXL, 0);
          stop(FRONT_DXL, 0); 
          musicStop();
          
          updateDesiredPosition(FRONT_DXL, UP, ONE_ROTATION);
          delay(TIME_OUT);
          state = WAIT_FLAG;
        }
        else
        {
          move(ROOM_DXL, DOWN, MOTOR_SPEED/2);
          move(FRONT_DXL, DOWN, MOTOR_SPEED/2);
          musicStart();
        }
        break;
    }
  }
}
