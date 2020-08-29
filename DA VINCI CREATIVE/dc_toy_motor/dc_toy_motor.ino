/* Authors: Taehun Lim (Darby) */

const int sensor_pin = 40;     // the number of the pushbutton pin

int sensor_state = 0;

const int motor_pin_1 = 50;
const int motor_pin_2 = 51;
const int motor_pin_3 = 52;
const int motor_pin_4 = 53;

void setup() {
  Serial.begin(9600);
//  while(!Serial); // Wait for Opening Serial Monitor

  // Set up the built-in LED pin as an output:
  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);
  pinMode(motor_pin_3, OUTPUT);
  pinMode(motor_pin_4, OUTPUT);

  pinMode(sensor_pin, INPUT);
}

void loop()
{
  sensor_state = digitalRead(sensor_pin);
  if (sensor_state == HIGH)
  {
    digitalWrite(motor_pin_1, HIGH);
    digitalWrite(motor_pin_2, HIGH);
    digitalWrite(motor_pin_3, HIGH);
    digitalWrite(motor_pin_4, HIGH);
    Serial.println("motor_on");
    delay(1000);
  }
  else
  {
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, LOW);
    digitalWrite(motor_pin_3, LOW);
    digitalWrite(motor_pin_4, LOW);
    Serial.println("motor_off");
    delay(1000);
  }
}
