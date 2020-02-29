/* Authors: Darby Lim */
#include <MsTimer2.h>
#include <DynamixelShield.h>

#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial

// #define GET_MOTION
 #define MOTION_PLAY

const uint8_t DXL_CNT = 4;
const uint8_t DXL_ID[DXL_CNT] = {11, 12, 13, 14};

void timer_init()
{
  int ms = 8;
  MsTimer2::set(ms, run);
  MsTimer2::start();
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  timer_init();
}

void loop() {}
void run()
{

}
