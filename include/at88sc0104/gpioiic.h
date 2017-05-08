#ifndef _GPIOIIC_H_
#define _GPIOIIC_H_

#define ACK  0x00
#define NACK 0x01

#include "common.h"

#include "GpioApi.h"



void Init_Clock();


void I2C_Start();
void I2C_Stop();

void  I2C_SendByte(unsigned char  c);
unsigned char I2C_ReceiveByte();

void  I2C_SendACK(unsigned char  a);
unsigned char I2C_WaitACK();
void  I2C_WaitIdle(unsigned char  n);

void set_user_zone(unsigned char  n);

void read_user_zone(unsigned char  addr1, unsigned char  addr2, unsigned char  n, unsigned char  *data);
void write_user_zone(unsigned char  addr1, unsigned char  addr2, unsigned char  n, unsigned char  *data);

void system_read(unsigned char  addr, unsigned char  n, unsigned char  *data);
void system_write(unsigned char  addr, unsigned char  n, unsigned char  *data);

void verify_read_password(unsigned char  n, unsigned char  * pw);
void verify_write_password(unsigned char  n, unsigned char  * pw);

void verify_authentication(unsigned char  n, unsigned char  * au);
void verify_crypto(unsigned char  n, unsigned char  * cr);

void Unlock_Config_Zone();

void Read_Fuse_Status();


#endif

