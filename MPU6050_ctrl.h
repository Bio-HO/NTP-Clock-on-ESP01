#ifndef MPU6050_CTRL_H
#define MPU6050_CTRL_H

// MPU-6000 I2C address is 0x68(104)
#define Addr 0x68

void MPU6050_rst();
void MPU6050_ini();
void MPU6050_read(double* Ax, double* Ay, double* Az, double* Gx, double* Gy, double* Gz, double* Temp, int* error);

#endif
