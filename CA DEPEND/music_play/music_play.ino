/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins. 
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
//  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

bool music_on = false;
  
void setup() {
  SerialUSB.begin(9600);
//  while(!SerialUSB);
  Serial1.begin(9600);
  SerialUSB.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
     SerialUSB.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  SerialUSB.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    SerialUSB.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // list files
//  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(5,5);

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  randomSeed(analogRead(0));
}

void loop() {
  while(Serial1.available())
  {
    char data = Serial1.read();
    SerialUSB.println(data);

    if (data == 'A')
    {
      if (music_on == false)
      {
        if (random(1, 3) == 1)
          musicPlayer.startPlayingFile("/test1.mp3");
        else if (random(1, 3) == 2)
          musicPlayer.startPlayingFile("/test2.mp3");
        else
          musicPlayer.startPlayingFile("/test3.mp3");
  
        music_on = true;
      }
    }
    else if (data == 'B')
    {
      musicPlayer.stopPlaying();
      music_on = false;
    }
  }  
}


/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       SerialUSB.print('\t');
     }
     SerialUSB.print(entry.name());
     if (entry.isDirectory()) {
       SerialUSB.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       SerialUSB.print("\t\t");
       SerialUSB.println(entry.size(), DEC);
     }
     entry.close();
   }
}

