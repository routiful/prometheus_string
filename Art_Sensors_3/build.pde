ArrayList<UVertexList> vvl;

UVertexList vl, vl2;
UGeo geo;
UGeo geo1;
UGeo geo2;

int ave_leaf =3; // leaf count pram
float x_cod, y_cod;
int check, vol;
float[] posX = new float[10];
float[] posY = new float[10];
float[] posZ = new float[10];
float[] rosX = new float[10];
float[] rosY = new float[10];
float[] rosZ = new float[10];


void build(int depth, int count, int tempC, int UV, int light, int Mic, int RH) {
  int leaf_count = count/ave_leaf; 
  if(leaf_count >10) leaf_count = 10;
  geo=UGeoGenerator.geodesicSphere(15, 2);
  posX[0] = 0;
   posY[0] = 0;
    posZ[0] = 7.5;
  geo.translate(0 , 0 , posZ[0] );
  geo1= UGeoGenerator.meshBox(150,150,3,2) ;
  
  geo.add(geo1);
  
  

for(int i=0; i<leaf_count-1; i++){

  if(i==0){
  posX[1] = -tempC;
   posY[1] = -tempC;
}else if(i==1){
  posX[2] = UV/10;
   posY[2] = UV/10;
}
else if(i==2){
  posX[3] = light/10;
   posY[3] = light/10;
}else if(i==3){
  posX[4] = -RH;
   posY[4] = -RH;
}
else{
    posX[i+1] =random(-50,50);
   posY[i+1] = random(-50,50);}
   posZ[i+1] =7.5;
   geo1=UGeoGenerator.geodesicSphere(15, 2);
  geo1.translate(posX[i+1] , posY[i+1],posZ[i+1]);
  geo.add(geo1);

}
 for(int i=0; i<leaf_count; i++)
{
 rosX[i] = 0;
} 
  geo.add(geo1);
  geo.regenerateFaceData();;
 geo.writeSTL(sketchPath+"/"+UFile.noExt(filename)+".stl");

}

int cnt=0;

void deform() {
    UFace f=UMB.rnd(geo.getF());
    
    UVertex fv=f.centroid().copy().mult(0.1);
    fv.add(frameCount/1000,0,0);
    
    float force=sin(noise(fv.x,fv.y,0)*PI);
    force*=random(100)>80 ? -random(10,30) : random(10,30);
    
//    f.translate(random(-10,10) ,0 ,random(-10,10) );
    f.translate(new UVertex(f.normal().copy().mult(force)));
//    geo.regenerateFaceData(geo.getF().indexOf(f));
    
    if(cnt>0 && (cnt++)%30==0) USubdivision.subdivide(geo,UMB.SUBDIVMIDEDGES);
}


void grow(int depth, int count, int tempC, int UV, int light, int Mic, int RH) {
  int leaf_count = count/ave_leaf;
  if(leaf_count >10) leaf_count = 10;
  int ch1, ch2;
  
  for(int i=0; i<leaf_count; i++){
 check = (int)random(0, 9);
 vol = (int)random(15, 20);
 
 if(Mic < 500)
 { ch1 = 7; ch2 = 1;}
 else {ch1 = 4; ch2 = 2;}
  
 posX[i] = posX[i] + random(-10, 10);
 posY[i] = posY[i] + random(-10, 10);
 posZ[i] = posZ[i]+10;
 rosX[i] = rosX[i] + random(0.01, 0.01+depth/500);
 if(check <ch1)
 {
  geo1=UGeoGenerator.geodesicSphere(vol, 2);
  geo1.translate(posX[i] ,posY[i] , posZ[i]);
  geo1.rotX(rosX[i]);
  geo.add(geo1);
 }
 else if(check >=ch1 && check < ch1+ch2 )
 {
  geo1=UGeoGenerator.geodesicSphere(vol, 2);
  geo1.translate(posX[i] ,posY[i] , posZ[i]);
  geo1.rotX(rosX[i]);
  geo.add(geo1);
  
  geo1 = UGeoGenerator.meshBox(vol,vol,vol/2,2) ;
  geo1.translate(posX[i] ,posY[i] , posZ[i]);
  geo1.rotX(rosX[i]);
  geo.add(geo1);
 }
 else if(check >=ch1+ch2 && check < ch1+ch2*2 )
 {
  geo1=UGeoGenerator.meshBox(vol,vol,vol,2) ;
  geo1.translate(posX[i] ,posY[i] , posZ[i]);
  geo1.rotX(rosX[i]);
  geo.add(geo1);
  }
  else if(check >=ch1+ch2*2 && check < ch1+ch2*3 )
 {
  geo1=UGeoGenerator.meshBox(vol,vol,vol,2) ;
  geo1.rotX(0.5);
  geo1.rotY(0.5);
  geo1.rotZ(0.5);
  geo1.translate(posX[i] ,posY[i] , posZ[i]);
  geo1.rotX(rosX[i]);
  geo.add(geo1);
  }
 }
  
  
  geo.writeSTL(sketchPath+"/"+UFile.noExt(filename)+".stl");
}
