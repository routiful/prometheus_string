// ModelbuilderMk2----------------------------------------------------
import unlekker.mb2.geo.*;
import unlekker.mb2.util.*;
import unlekker.data.*;
import unlekker.mb2.externals.*;
import ec.util.*;

// Finger tracker using Kinect v1----------------------------------------------------
import fingertracker.*;
import SimpleOpenNI.*;

// Serial comm to arduino----------------------------------------------------
import processing.serial.*;

// Set Constant value
String FILENAME = "darby";
String SERIAL_PORT = "com3";

color TRACK_COLOR = color(165,104,88);
float INIT_DIST_BTW_COLORS = 500.0; 

int DISTANCE_FOR_FINGER_TRACKING = 625;

int WHEN_RESET = 5; // count
int TIME_COUNT_THRESHOLD = 10; // sec

int WAIT_FOR_INIT = 5; // sec

float LATITUDE = 38.0;   // degree (-90  ~ 90)
float LONGITUDE = 127.0;  // degree (-180 ~ 180)

// Set sensor
boolean connectedKinect = false;
boolean connectedArduino = false;

// Load main classes----------------------------------------------------
UNav3D nav;
FingerTracker fingers;
SimpleOpenNI  kinect;

// Variables for finger tracking----------------------------------------------------
int dist_finger_traking_threshold = DISTANCE_FOR_FINGER_TRACKING;
PImage currentFrame;

// Variables for serial comm----------------------------------------------------
Serial commPort;
char HEADER = 'H';

// Variables sensor data-----------------------------------------------------
color trackColor = TRACK_COLOR;// Define trackColor
int color_count = 0;
int finger_count = 0;

int tempC = 0;
int RH = 0;
int UV = 0; 
int Mic = 0;
int light = 0;

int temp_init = 0;
int RH_init = 0;
int UV_init = 0;
int light_init = 0;

// Etc-----------------------------------------------------
int reset_count = 0;
int tTime = millis();
int time_count = 1;

int[] D1 = new int[40];
int[] D2 = new int[40];
int[] D3 = new int[40];
int[] D4 = new int[40];
int[] D5 = new int[40];
int[] D6 = new int[40];
int[] D7 = new int[40];

float world_rot_x, world_rot_y;

void setup() 
{  
  size(800, 800, P3D);
  currentFrame = createImage(640, 480, RGB); // Init size of image

  UMB.setPApplet(this);
  nav=new UNav3D();

  // Init view----------------------------------------------------
  nav.rot.set(radians(90), radians(0), radians(0));
  nav.trans.set(500, 700, 0);
  
  if (connectedKinect)
  {
    // Init kinect----------------------------------------------------
    kinect = new SimpleOpenNI(this);
    kinect.enableDepth();
    kinect.setMirror(true); // Don't use skeleton data??
    kinect.enableRGB();

    // Init finger tracking----------------------------------------------------
    fingers = new FingerTracker(this, 640, 480);  
    smooth ();
    fingers.setMeltFactor(100);
  }

  if (connectedArduino)
  {
    // Init commPort----------------------------------------------------
    commPort  = new Serial(this, SERIAL_PORT, 57600);
  }

  build((int)random(0, 50), (int)random(0, 100), (int)random(20, 30), (int)random(0, 15), (int)random(0, 1024), (int)random(0, 1024), (int)random(0, 100));

  println("END SETUP");
}

void draw() 
{
  background(0);

  if (connectedKinect) // Find color_count and finger_count
  {
    // Update Kinect data----------------------------------------------------
    kinect.update();
    PImage depthImage = kinect.depthImage();
    currentFrame = kinect.rgbImage();
    fingers.setThreshold(dist_finger_traking_threshold);
  
    color_count = 0;

    float worldRecord_of_dist = INIT_DIST_BTW_COLORS;

    // Begin loop to walk through every pixel
    for (int x = 0; x < currentFrame.width; x++) 
    {
      for (int y = 0; y < currentFrame.height; y++) 
      {
        int loc = x + y * currentFrame.width;
        // What is current color
        color currentColor = currentFrame.pixels[loc];

        float r1 = red(currentColor);
        float g1 = green(currentColor);
        float b1 = blue(currentColor);

        float r2 = red(trackColor);
        float g2 = green(trackColor);
        float b2 = blue(trackColor);

        // Using euclidean distance to compare colors
        // We are using the dist() function to compare the current color 
        // with the color we are tracking.
        float dist_btw_colors = dist(r1,g1,b1,r2,g2,b2);

        // If current color is more similar to tracked color than
        // closest color, save current location and current difference
        if (dist_btw_colors < worldRecord_of_dist) 
        {
          color_count++;
          worldRecord_of_dist = dist_btw_colors;
        }
      }
    }
  
    int[] depthMap = kinect.depthMap();
    fingers.update(depthMap);
    stroke(0,255,0);
    finger_count = fingers.getNumFingers();
  }

  if ((millis() - tTime) > 1000)
  {
    if (time_count%TIME_COUNT_THRESHOLD == 0)
    {
      grow(finger_count, color_count, tempC, UV, light, Mic, RH);
    }

    time_count++;
    tTime = millis();
  }
  showDataValue();

  lights();
  nav.doTransforms();

  noStroke();
  fill(255);
  geo.draw();
}

void serialEvent(Serial p) 
{
  String message = commPort.readStringUntil('\n');  

  if (message != null) 
  {
    // try catch function because of possible garbage in received data
    try 
    {
      String[] elements = message.split(",");//splitTokens(message);

      if((elements[0].charAt(0) == HEADER) && (elements.length >= 1) && (time_count > WAIT_FOR_INIT))
      {
        // Maybe, we can use constrain(), map()
        RH    = int(elements[1]);// - RH_init;
        tempC = int(elements[2]);// - temp_init; 
        light = int(elements[3]);// - light_init;
        UV    = int(elements[4]);// - UV_init;
        Mic   = int(elements[5]);
        
        for (int j = 39; j !=0 ; j--)
        {
          D1[j] = D1[j-1];
          D2[j] = D2[j-1];
          D3[j] = D3[j-1];
          D4[j] = D4[j-1];
          D5[j] = D5[j-1];
          D6[j] = D6[j-1];
          D7[j] = D7[j-1];
        }
        
        D1[0] = finger_count;
        D2[0] = color_count;
        D3[0] = tempC;
        D4[0] = UV;
        D5[0] = Mic;
        D6[0] = RH;
        D7[0] = light; 
      }
      /*
      // Init after 10 seconds 
      else if((elements[0].charAt(0) == HEADER) && (elements.length >= 1) && (time_count == WAIT_FOR_INIT))
      {
        RH_init    = int(elements[1]);
        temp_init  = int(elements[2]); 
        light_init = int(elements[3]);
        UV_init    = int(elements[4]);
        Mic        = int(elements[5]);

        build(finger_count, color_count, temp_init, UV_init, light_init, Mic, RH_init);
      }
      */
    }
    catch (Exception e) 
    {
    }
  }
}

void showDataValue()
{    
  fill(255,255,255);
  text("finger", 10, 20);
  text("color", 50, 20);
  text("tempC", 90, 20);
  text("UV", 130, 20);
  text("MIC", 170, 20);
  text("RH", 210, 20);
  text("light", 250, 20);
  text("time", 290, 20);
  
  for(int j = 0; j < 40; j++)
  {    
    fill(255,0,0); 
    text(D1[j], 10, 35*(j+1));
    text(D2[j], 50, 35*(j+1));
    text(D3[j], 90, 35*(j+1));
    text(D4[j], 130, 35*(j+1));
    text(D5[j], 170, 35*(j+1));
    text(D6[j], 210, 35*(j+1));
    text(D7[j], 250, 35*(j+1));
    text(time_count, 290, 35);
  }
}

void keyPressed() 
{
  if (key=='s' || key=='S') 
  {
    renderTiles();
  }

  else if (key==' ') 
  {
  //  build();
    colorMesh();
  }

  else if (key=='a' || key=='A') 
  {
    if (connectedArduino)
    {
      grow(finger_count, color_count, tempC, UV, light, Mic, RH);
    }
    else
    {
      finger_count = (int)random(0, 50);
      color_count  = (int)random(0, 100);
      tempC        = (int)random(20, 30);
      UV           = (int)random(0, 15);
      light        = (int)random(0, 1024);
      Mic          = (int)random(0, 1024);
      RH           = (int)random(0, 100);

      for (int j = 39; j !=0 ; j--)
      {
        D1[j] = D1[j-1];
        D2[j] = D2[j-1];
        D3[j] = D3[j-1];
        D4[j] = D4[j-1];
        D5[j] = D5[j-1];
        D6[j] = D6[j-1];
        D7[j] = D7[j-1];
      }
      
      D1[0] = finger_count;
      D2[0] = color_count;
      D3[0] = tempC;
      D4[0] = UV;
      D5[0] = Mic;
      D6[0] = RH;
      D7[0] = light; 

      time_count++;

      grow(finger_count, color_count, tempC, UV, light, Mic, RH);
    } 
  } 
}
