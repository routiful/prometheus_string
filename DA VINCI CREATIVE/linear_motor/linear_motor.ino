int E1 = 10;      // 1번(A) 모터 Enable
int E2 = 11;      // 2번(B) 모터 Enable
int M1 = 12;      // 1번(A) 모터 PWM
int M2 = 13;      // 2번(B) 모터 PWM

const int sensor_pin = A0;
const int upper_button_pin = 7;
const int lower_button_pin = 8;

int sensor_value = 0;
int output_value = 0;

int button_state = 0;

const int GET_SENSOR_DATA = 0;
const int MOVE_MOTOR_TO_FORWARD = 1;
const int MOVE_MOTOR_TO_BACKWARD = 2;

void setup()
{
  Serial.begin(9600);
  while(!Serial);

  pinMode(M1, OUTPUT);      // 출력핀 설정
  pinMode(M2, OUTPUT);

  pinMode(sensor_pin, INPUT);

  pinMode(upper_button_pin, INPUT_PULLUP);
  pinMode(lower_button_pin, INPUT_PULLUP);
}

void loop()
{
  static int state = GET_SENSOR_DATA;
  static int cnt = 0;

  switch(state)
  {
    case GET_SENSOR_DATA:
        sensor_value = analogRead(sensor_pin);
        output_value = map(sensor_value, 0, 1023, 0, 255);
        Serial.println(output_value);
        if (output_value >= 100)
        {
          Serial.println("Sensor ON");
          state = MOVE_MOTOR_TO_FORWARD;
        }
      break;

    case MOVE_MOTOR_TO_FORWARD:
        button_state = digitalRead(lower_button_pin);
        Serial.print("lower_button_state : ");
        Serial.println(button_state);
        if (button_state == HIGH)
        {
          move_forward(255);
          Serial.println("forward");
          state = MOVE_MOTOR_TO_FORWARD;
        }
        else
        {
          Serial.println("stop");
          stop();
          state = MOVE_MOTOR_TO_BACKWARD;
        }
      break;

    case MOVE_MOTOR_TO_BACKWARD:
        button_state = digitalRead(upper_button_pin);
        Serial.print("upper_button_state : ");
        Serial.println(button_state);
        if (button_state == HIGH)
        {
          Serial.println("backward");
          move_backward(255);
          state = MOVE_MOTOR_TO_BACKWARD;
        }
        else
        {
          Serial.println("stop");
          stop();
          if (cnt == 0)
          {
            state = MOVE_MOTOR_TO_FORWARD;
            cnt++;
          }
          else
          {
            state = GET_SENSOR_DATA;
            cnt = 0;
          }
        }
      break;

    default:
      stop();
      break;
  }
//  sensor_value = analogRead(sensor_pin);
//  output_value = map(sensor_value, 0, 1023, 0, 255);
//  if (output_value >= 100)
//  {
//    Serial.println(output_value);
//    move_forward(3000, 255);
//    stop(5000);
//    move_backward(6000, 255);
//    stop(5000);
//    move_forward(3000, 255);
//    stop(5000);
//  }
}

void stop(int time)
{
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  analogWrite(E1, 0);
  analogWrite(E2, 0);
  delay(time);
}

void stop()
{
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  analogWrite(E1, 0);
  analogWrite(E2, 0);
}

void move_forward(int time, int value)
{
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  analogWrite(E1, value);
  analogWrite(E2, value);
  delay(time);
}

void move_forward(int value)
{
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  analogWrite(E1, value);
  analogWrite(E2, value);
}

void move_backward(int time, int value)
{
  digitalWrite(M1, HIGH);
  digitalWrite(M2, HIGH);
  analogWrite(E1, value);
  analogWrite(E2, value);
  delay(time);
}

void move_backward(int value)
{
  digitalWrite(M1, HIGH);
  digitalWrite(M2, HIGH);
  analogWrite(E1, value);
  analogWrite(E2, value);
}
