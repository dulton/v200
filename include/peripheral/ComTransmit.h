#ifndef _COM_TRANSMIT_H_
#define _COM_TRANSMIT_H_

#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {
	unsigned long 		baudrate;	/*波特率*/
	unsigned char 		databit;		/*数据位，取值5、6、7、8*/
	unsigned char 		parity;		/*奇偶校验位，取值'n':无校验'o':奇校验'e'偶校验*/
	unsigned char 		stopbit;		/*停止位，取值1,2*/
	unsigned char 		reserve;		/*保留字节*/
}Terminal;

typedef struct
{	
	int  sysSpeed;
	unsigned long  userSpeed;
}speedPara;

typedef enum
{
	READ_TIMEOUT_F = 0,
	WRITE_TIMEOUT_F
}SELECT_TIMEOUT_F;


typedef enum
{
   BAUTRATE_1200 = 0,
   BAUTRATE_2400,
   BAUTRATE_4800,
   BAUTRATE_9600,
   BAUTRATE_19200,
   BAUTRATE_38400,
   BAUTRATE_57600,
   BAUTRATE_115200,

}SERIALBAUTRATE_E;


class CComTransmit
{
private:
	//	
	FILE 	*m_fp;	
	int 		m_fd;	
	struct timeval Rtimeout;
	struct timeval Wtimeout;
public:
	//
	CComTransmit();
	~CComTransmit();
	int OpenCom(const char *deviceName);
	void CloseCom();	
	int SetComSpeed(unsigned int speed);
	int SetComParity(int databits,int stopbits,int parity,int flowctrl);
	int SetComRawMode();
	int SetComSelectTimeOut(int sec,int usec,SELECT_TIMEOUT_F RWflag);//0--read,1--wirte
	int IOFlush();
	int ReadCom(char *ReadBuffer,int size,int *retSize);
	int WriteCom(char *WriteBuffer,int size);	
	void ConvertCR2LF(int  Switch);
	int GetPortFD();
	FILE* GetPortFP();
protected:
	//
};

#endif//_COM_TRANSMIT_H_

