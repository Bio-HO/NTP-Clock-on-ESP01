#include "MPU6050_ctrl.h"
#include <Wire.h>

void MPU6050_rst(){
  Wire.begin(2,0);  // for ESP01
  Wire.beginTransmission(Addr);
  Wire.write(107);
  Wire.write(0x80); // Device reset
  Wire.endTransmission();
}

void MPU6050_ini(){
  Wire.begin(2,0);  // for ESP01
  
  Wire.beginTransmission(Addr); 
  Wire.write(0x1B); // Select gyroscope configuration register
  Wire.write(0x18); // Full scale range = 2000 dps
  Wire.endTransmission();
  
  Wire.beginTransmission(Addr);  
  Wire.write(0x1C); // Select accelerometer configuration register  
  Wire.write(0x18); // Full scale range = +/-16g
  Wire.endTransmission();
  
  Wire.beginTransmission(Addr); 
  Wire.write(0x6B); // Select power management register
  Wire.write(0x01); // PLL with xGyro reference
  Wire.endTransmission();
}

void MPU6050_read(double* Ax, double* Ay, double* Az, double* Gx, double* Gy, double* Gz, double* Temp, int* error){

  unsigned int data[6];
  int xch,ych,zch;
  double xch_d,ych_d,zch_d;

  Wire.beginTransmission(Addr);
  Wire.write(0x3B); // Accelrator data
  Wire.endTransmission();
  Wire.requestFrom(Addr, 6);

  if(Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read(); 
  }
  
  // Convert the data
  if(data[0] >= 128){
    xch = data[0] * 256 + data[1] - 65536;
  }
  else{
    xch = data[0] * 256 + data[1];
  }
  if(data[2] >= 128){
    ych = data[2] * 256 + data[3] - 65536;
  }
  else{
    ych = data[2] * 256 + data[3];
  }
  if(data[4] >= 128){
    zch = data[4] * 256 + data[5] - 65536;
  }
  else{
    zch = data[4] * 256 + data[5];
  }

  *Ax = (double)xch/2048;
  *Ay = (double)ych/2048;
  *Az = (double)zch/2048;

  Wire.beginTransmission(Addr);
  Wire.write(0x43); // Gyro data
  Wire.endTransmission();
  Wire.requestFrom(Addr, 6);

  if(Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read(); 
  }
  
  // Convert the data
  if(data[0] >= 128){
    xch = data[0] * 256 + data[1] - 65536;
  }
  else{
    xch = data[0] * 256 + data[1];
  }
  if(data[2] >= 128){
    ych = data[2] * 256 + data[3] - 65536;
  }
  else{
    ych = data[2] * 256 + data[3];
  }
  if(data[4] >= 128){
    zch = data[4] * 256 + data[5] - 65536;
  }
  else{
    zch = data[4] * 256 + data[5];
  }

  *Gx = (double)xch/16;
  *Gy = (double)ych/16;
  *Gz = (double)zch/16;


  Wire.beginTransmission(Addr);
  Wire.write(0x41); // Temp data
  Wire.endTransmission();
  Wire.requestFrom(Addr, 2);

  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  if(data[0] >= 128){
    xch = data[0] * 256 + data[1] - 65536;
  }
  else{
    xch = data[0] * 256 + data[1];
  }

  *Temp = (double)xch/340+36.53;

  // error check
  *error = 0;
  if((*Ax == 0)&&(*Ay == 0)&&(*Az == 0)){
    *error = 1;
  }
  if((*Ax >= 5)||(*Ay >= 5)||(*Az >= 5)){
    *error = 2;
  }
}
