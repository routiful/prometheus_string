import unlekker.mb2.geo.*;
import unlekker.mb2.util.*;
import unlekker.data.*;
import unlekker.mb2.externals.*;
import ec.util.*;
import fingertracker.*;
import SimpleOpenNI.*;
import processing.serial.*;

String filename="aapl2008";
/*
 ModelbuilderMk2 - UTileRendering.pde
 Marius Watz - http://workshop.evolutionzone.com
 
 Demonstrates how to use UTileRenderer to render 
 high-res images from realtime graphics. 
 
 */

UNav3D nav;
FingerTracker fingers;
SimpleOpenNI  kinect;
int reset = 5;
int threshold = 625;
int col_count, finger_count, ave_col, time_count=0, reset_count=0;
PImage currentFrame;
color trackColor;
Serial commPort ;

String row_data;
char HEADER = 'H';
float y, h;
int tempC, tempF, RH, UV, Mic, light;
int temp_init, RH_init, UV_init, light_init;


int[] D1 = new int[40];
int[] D2 = new int[40];
int[] D3 = new int[40];
int[] D4 = new int[40];
int[] D5 = new int[40];
int[] D6 = new int[40];
int[] D7 = new int[40];

public void setup() {
  
  size(800, 800, P3D);
  UMB.setPApplet(this);
  nav=new UNav3D();
  nav.rot.set(radians(90), radians(10), radians(10));
  nav.trans.set(50, 50, 400);
  
  kinect = new SimpleOpenNI(this);
  kinect.enableDepth();
  kinect.setMirror(true); 
  fingers = new FingerTracker(this, 640, 480);
  kinect.enableRGB();
  trackColor = color (120,160,60);
  smooth ();
  fingers.setMeltFactor(100);
//  handPositions = new ArrayList(); 
  currentFrame = createImage (640,480, RGB);
  commPort  = new Serial(this, "com8", 9600);
  //size(kinect.depthWidth()*2, kinect.depthHeight());
  //stroke(255,0,0); 
  //strokeWeight(2); 
 // build();
// if(time_count >60)
// {
build(finger_count, ave_col, temp_init, UV_init, light_init, Mic, RH_init);
// time_count = 6;
// }
 // colorMesh();
 /*if(time_count ==7)
 {
   lstart(ave_col);
 }*/
}

public void draw() {
  background(0);
 kinect.update();
  PImage depthImage = kinect.depthImage();
 // image(depthImage,0,0);
  currentFrame = kinect.rgbImage();
 // image(currentFrame,640,0);
//  strokeWeight(10);
  float worldRecord = 500;
  fingers.setThreshold(threshold);
  
  // XY coordinate of closest color
  int closestX = 0;
  int closestY = 0;
  col_count =0;
   // Begin loop to walk through every pixel
   for (int x = 0; x < currentFrame.width; x ++ ) {
   for (int y = 0; y < currentFrame.height; y ++ ) {
   int loc = x + y*currentFrame.width;
   // What is current color
   color currentColor = currentFrame.pixels[loc];
   float r1 = red(currentColor);
   float g1 = green(currentColor);
   float b1 = blue(currentColor);
   float r2 = red(trackColor);
   float g2 = green(trackColor);
   float b2 = blue(trackColor);
  
  // Using euclidean distance to compare colors
   float d = dist(r1,g1,b1,r2,g2,b2); // We are using the dist( ) function to compare the current color with the color we are tracking.
  
  // If current color is more similar to tracked color than
   // closest color, save current location and current difference
   if (d < worldRecord) {
     col_count++;
   worldRecord = d;
   closestX = x;
   closestY = y;
   }
   }
   }
   
  // We only consider the color found if its color distance is less than 10.
 // This threshold of 10 is arbitrary and you can adjust this number depending on how accurate you require the tracking to be.
 /*if (worldRecord < 100) {
 // Draw a circle at the tracked pixel
 fill(trackColor);
 strokeWeight(4.0);
 stroke(0);
 ellipse(closestX+640,closestY,16,16);
 }*/
 
  int[] depthMap = kinect.depthMap();
  fingers.update(depthMap);
  stroke(0,255,0);
  for (int i = 0; i < fingers.getNumFingers(); i++) {
    PVector position = fingers.getFinger(i);
 //   ellipse(position.x - 5, position.y -5, 10, 10);
  }
  finger_count = fingers.getNumFingers();
  
//  println("Thres="+threshold); 
  //println("y="+y); 

  fill(255,0,0); 
  for(int j = 39; j!=0; j--){
  D1[j] = D1[j-1];
  D2[j] = D2[j-1];
  D3[j] = D3[j-1];
  D4[j] = D4[j-1];
  D5[j] = D5[j-1];
  D6[j] = D6[j-1];
  D7[j] = D7[j-1];
  }
  
  D1[0] = finger_count;
  D2[0] = ave_col;
  D3[0] = tempC;
  D4[0] = UV;
  D5[0] = Mic;
  D6[0] = RH;
  D7[0] = light;
  
  for(int j = 0; j<40; j++){
  fill(255,0,0); 
  text(D1[j], 10, 20*(j+1));
  fill(255,0,0); 
  text(D2[j], 50, 20*(j+1));
  fill(255,0,0); 
  text(D3[j], 90, 20*(j+1));
  fill(255,0,0); 
  text(D4[j], 130, 20*(j+1));
  fill(255,0,0); 
  text(D5[j], 170, 20*(j+1));
  fill(255,0,0); 
  text(D6[j], 210, 20*(j+1));
  fill(255,0,0); 
  text(D7[j], 250, 20*(j+1));
  }
  if(time_count ==1800)
  {
    grow(finger_count, ave_col, tempC, UV, light, Mic, RH);
    time_count = 11;
    reset_count ++;
  }
  if(reset_count ==reset)
  {
    build(finger_count, ave_col, temp_init, UV_init, light_init, Mic, RH_init);
    reset_count =0;
  }
  // if tiler exists, see if we are done tiling. if so, set tiler to null. 
/*  if (tiler!=null && tiler.done) {
    tiler=null;
  } else if(tiler==null) {
    // the credit text should only be drawn when not tiling
    drawCredit();
  }*/

  translate(width/2, height/2);
  lights();
  nav.doTransforms();

  noStroke();
  fill(255);
  geo.draw();
//   geo1.draw();
  
}

 void serialEvent(Serial p) {
// get message till line break (ASCII > 13)
String message = commPort.readStringUntil('\n');  


if (message != null) {
// try catch function because of possible garbage in received data
try {
//print(message);
String[] elements = message.split(",");//splitTokens(message);

if(elements[0].charAt(0) == HEADER && elements.length >= 1 && time_count >10) // check validity
{
RH =  int(elements[1])-RH_init;
tempC = int(elements[2])-temp_init; 
UV = int(elements[4])-UV_init;
light = int(elements[3])-light_init;
Mic = int(elements[5]);
}
else if(elements[0].charAt(0) == HEADER && elements.length >= 1 && time_count ==10) // check validity
{
RH_init =  int(elements[1]);
temp_init = int(elements[2]); 
UV_init = int(elements[4]);
light_init = int(elements[3]);
ave_col = (ave_col+col_count)/2;
build(finger_count, ave_col, temp_init, UV_init, light_init, Mic, RH_init);
}


print("tempC: "+tempC);
print("UV: "+UV);
print("light: "+light);
print("Mic: "+Mic);
println("RH: "+RH);
delay(1000);
time_count++;
print("color: "+ave_col);
print("finger: "+finger_count);
println("time: "+time_count);

 

}
catch (Exception e) {
}
}
}

void mousePressed() {
  grow(finger_count, ave_col, tempC, UV, light, Mic, RH);
}

