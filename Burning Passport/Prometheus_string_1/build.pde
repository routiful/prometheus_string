// https://github.com/mariuswatz/modelbuilderMk2/blob/master/export/modelbuilderMk2/src-modelbuilderMk2/unlekker/mb2/geo/UGeoGenerator.java

UGeo geo;
UGeo geo1;

float stem_cnt = 0;
int SPHERE_POLYGON = 0;

float[] posX = new float[10];
float[] posY = new float[10];
float[] posZ = new float[10];

float[] rotX = new float[10];
float[] rotY = new float[10];
float[] rotZ = new float[10];

void deform(UGeo get_geo, float latitude, float longitude, float offset) 
{
  int cnt = 0;
  UFace f = UMB.rnd(get_geo.getF());

  UVertex fv = f.centroid().copy().mult(0.1);
  fv.add(frameCount/1000,0,0);

  float force = sin(noise(latitude * offset, longitude * offset, 0)*PI);
  force *= random(100)>80 ? -random(10,30) : random(10,30);

  // f.translate(random(-10,10) ,0 ,random(-10,10) );
  f.translate(new UVertex(f.normal().copy().mult(force)));
  // get_geo.regenerateFaceData(get_geo.getF().indexOf(f));

  if (cnt > 0 && (cnt++) % 30 == 0) 
    USubdivision.subdivide(get_geo, UMB.SUBDIVMIDEDGES);
}

void build(int finger, int color_count, int tempC, int UV, int light, int Mic, int RH) 
{
  geo = UGeoGenerator.geodesicSphere(15, SPHERE_POLYGON); // radius, polygon?
  //geo.translate(0.0, 0.0, 7.5);

  //geo= UGeoGenerator.meshBox(150, 150, 3, 2); // x, y, z, polygon  
  //geo.add(geo1);  // Add plate

  color_count = constrain(color_count, 0, 100);
  stem_cnt =2;// map(color_count, 0, 100, 0, 5);

  for (int i = 0; i < stem_cnt; i++)
  {
    if (i == 0)
    {
      posX[i] = map(tempC, 20, 30, -30.0, 30.0);
      posY[i] = map(tempC, 20, 30, -30.0, 30.0);
    }
    else if (i == 1)
    {
      posX[i] = map(UV, 0, 15, -30.0, 30.0);
      posY[i] = map(UV, 0, 15, -30.0, 30.0);
    }
    else if (i == 2)
    {
      posX[i] = map(light, 0, 1024, -30.0, 30.0);
      posY[i] = map(light, 0, 1024, -30.0, 30.0);
    }
    else if (i == 3)
    {
      posX[i] = map(RH, 0, 100, -30.0, 30.0);
      posY[i] = map(RH, 0, 100, -30.0, 30.0);
    }
    else
    {
      posX[i] = random(-30.0, 30.0);
      posY[i] = random(-30.0, 30.0);
    }

    posZ[i] = 7.5;

    geo1=UGeoGenerator.geodesicSphere(map(Mic, 0, 1024, 15, 20), SPHERE_POLYGON);
    geo1.translate(posX[i] , posY[i], posZ[i]);
    // println("posX = " + posX[i] + " posY = " + posY[i] + " posZ = " +  posZ[i]);

    if (random(-5.0, 5.0) > 0.0)
      deform(geo1, LATITUDE, LONGITUDE, random(-0.1, 0.1));

    geo.add(geo1);
  }

  geo.regenerateFaceData();
  geo.writeSTL(sketchPath+"/"+UFile.noExt(FILENAME)+".stl");
}

int grow_cnt = 2;

void grow(int finger, int color_count, int tempC, int UV, int light, int Mic, int RH) 
{
  int ch1, ch2;
  
  for (int i = 0; i < stem_cnt; i++)
  {
    float check, vol;

    check = map(UV, 0, 15, 0.0, 4.5) + random(0.0, 4.5); // Select object

    if (grow_cnt % 7 == 0)
      vol = map(light, 0, 1024, 15.0, 17.0) + random(0.0, 50.0);
    else
      vol = map(light, 0, 1024, 15.0, 17.0) + random(0.0, 10.0); // Volume 15~20mm

    if (Mic < (1024/2)) // ratio 7:1:1:1
    { 
      ch1 = 7;
      ch2 = 1;
    }
    else               // ratio 4:2:2:2
    {
      ch1 = 4; 
      ch2 = 2;
    }

    posX[i] = posX[i] + map(color_count, 0, 100, -5.0, 5.0) + random(-7.0, 7.0);
    posY[i] = posY[i] + map(finger, 0, 50, -5.0, 5.0) + random(-7.0, 7.0);
    posZ[i] = 7.5 * grow_cnt;
    // println("posX = " + posX[i] + " posY = " + posY[i] + " posZ = " +  posZ[i]);

    float f_RH = map(RH, 0, 100, 0, 0.5) + random(0, 0.5);
    rotX[i] = radians(random(0, f_RH));
    rotY[i] = radians(random(0, f_RH));
    rotZ[i] = radians(random(0, f_RH));

    if (check < ch1)
    {
      geo1 = UGeoGenerator.geodesicSphere(vol, SPHERE_POLYGON);

      geo1.translate(posX[i], posY[i], posZ[i]);
      geo1.rotX(rotX[i]);
      
      if (random(-5.0, 5.0) > 0.0)
        deform(geo1, LATITUDE, LONGITUDE, random(-0.1, 0.1));

      geo.add(geo1);
    }
    else if ((check >= ch1) && (check < ch1+ch2) )
    {
      geo1 = UGeoGenerator.geodesicSphere(vol, SPHERE_POLYGON);

      geo1.translate(posX[i], posY[i], posZ[i]);
      geo1.rotX(rotX[i]);

      if (random(-5.0, 5.0) > 0.0)
        deform(geo1, LATITUDE, LONGITUDE, random(-0.1, 0.1));

      geo.add(geo1);
      
      geo1 = UGeoGenerator.meshBox(vol, vol, vol/2, 2);

      geo1.translate(posX[i], posY[i], posZ[i]);
      geo1.rotX(rotX[i]);

      geo.add(geo1);
    }
    else if ((check >= ch1+ch2) && (check < ch1+ch2*2))
    {
      geo1 = UGeoGenerator.meshBox(vol, vol, vol, 2);

      geo1.translate(posX[i], posY[i], posZ[i]);
      geo1.rotX(rotX[i]);

      geo.add(geo1);
    }
    else if ((check >= ch1 + ch2*2) && (check < ch1 + ch2*3))
    {
      geo1 = UGeoGenerator.meshBox(vol, vol, vol, 2);

      geo1.rotX(rotX[i]);
      geo1.rotY(rotY[i]);
      geo1.rotZ(rotZ[i]);
      geo1.translate(posX[i], posY[i], posZ[i]);

      geo.add(geo1);
    }
  } 
  
  geo.writeSTL(sketchPath+"/"+UFile.noExt(FILENAME)+".stl");
  grow_cnt++;
}

