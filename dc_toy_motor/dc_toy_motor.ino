/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

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
