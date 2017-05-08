#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  /*Unix standard lib */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define GPIO_IOC_MAGIC            'g'
#define GPIO_UART1_SETOUTPUT	  _IOWR(GPIO_IOC_MAGIC, 4, int) 
#define GPIO_UART1_SETINPUT	      _IOWR(GPIO_IOC_MAGIC, 5, int) 

#define SERIAL_DEV   "/dev/ttyAMA1" 
#define BAUDRATE 115200

//enum{
//	FALSE,
//	TRUE
//};

int set_Parity(int fd, int databits, int stopbits, int parity);
void set_speed(int fd, int speed);
int OpenSerailDev();
int SerialRead(int fd,void *buff, size_t n);
int SerialWrite(int fd,const void *buf, size_t n);

#endif
