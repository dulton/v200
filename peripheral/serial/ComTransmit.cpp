#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#include "ComTransmit.h"

speedPara speed_atr[] = 
{
//	{B300,300},
//	{B600,600},
	{B1200,1200},
	{B2400,2400},
	{B4800,4800},
	{B9600,9600},
	{B19200,19200},
	{B38400,38400},
	{B57600,57600},
	{B115200,115200}			
};

CComTransmit::CComTransmit():m_fp(NULL),m_fd(-1)
{
	memset(&Rtimeout,0,sizeof(Rtimeout));
	memset(&Wtimeout,0,sizeof(Wtimeout));
}

CComTransmit::~CComTransmit()
{
	
}

int CComTransmit::OpenCom(const char * deviceName)
{
	assert(deviceName != NULL);
	if(m_fd > 0)
		close(m_fd);
	m_fd = open(deviceName,O_RDWR);
	printf("OpenCom deviceName %s:%d\n",deviceName,m_fd);
	if(m_fd < 0)
	{
		printf("Open Com [%s] failure!\n",deviceName);
		return -1;
	}
	
	m_fp = fdopen(m_fd, "r+b");	
	if(m_fp == NULL)
	{
		return -1;
	}

	return 0;
}

void CComTransmit::CloseCom()
{
	int ret = close(m_fd);
	if(ret < 0)
	{
		printf("close Com failure!\n");		
	}
	m_fp = NULL;
}

int CComTransmit::SetComSpeed(unsigned int speed)
{
	int databits =8;
	int parity = 0;
	int stopbits =1;

	unsigned int i, index=0;
	struct termios options;
	
	unsigned int speed_arr[] = {B0, B50,B75, B110, B134,B150 ,B200, B300, B600, B1200, B1800, B2400, 
					   B4800,B9600, B19200,B38400, B57600,B115200,B230400};/*baud rate table*/
	
	unsigned int name_arr[] = { 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 
					 19200, 38400, 57600, 115200, 230400};
	
	for(i=0;i<sizeof(name_arr)/sizeof(int);i++)
	{
		if(name_arr[i]== speed)
			break;
	}

	if(i >= sizeof(name_arr)/sizeof(int))
		return 1;

	//if(fd == rs232_fd)
	//	index = 0;
	//else
		index = 1;

	memset(&options, 0x00, sizeof(options));
	if (0 != tcgetattr(m_fd, &options)) 
	{
		return 3;
	}

	tcflush(m_fd, TCIOFLUSH); //清空缓存
	cfsetispeed(&options, speed_arr[i]);
	cfsetospeed(&options, speed_arr[i]);
   
	/* 
	CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能	
	CREAD--使能接收标志 
	CSIZE--字符长度掩码。取值为 CS5, CS6, CS7, 或 CS8 
	*/
	options.c_cflag |= (CLOCAL | CREAD);	// always should be set 
	options.c_cflag &= ~CSIZE;

	switch (stopbits) 
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
			
		case 2:
			options.c_cflag |= CSTOPB;
			break;
			
		default:
			return 4;
	}

	switch (databits) 
	{
		case 7:
			options.c_cflag |= CS7;
			break;
			
		case 8:
			options.c_cflag |= CS8;
			break;
			
		default:
			return 5;
	}

	options.c_cflag &= ~CRTSCTS;				// 不使用硬件流控制
	/*
	IXON--启用输出的 XON/XOFF 流控制
	IXOFF--启用输入的 XON/XOFF 流控制
	IXANY--允许任何字符来重新开始输出
	IGNCR--忽略输入中的回车
	*/
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	
	//options.c_iflag &= IGNCR; 				  // ignore CR 

	switch (parity) 
	{
		case 0:
			options.c_cflag &= ~PARENB; 			/* Clear parity enable */
			break;
			
		case 1:
			options.c_cflag |= PARENB;				/* Enable parity */
			options.c_cflag |= PARODD;				/* 设置为奇校验 */ 
			break;
			
		case 2:
			options.c_cflag |= PARENB;				/* Enable parity */
			options.c_cflag &= ~PARODD; 			/* 转换为偶校验 */
			break;
			
		case 3:
			options.c_cflag &= ~PARENB; 			/* Enable parity */
			options.c_cflag |= CSTOPB;
			break;
			
		case 4: 									/* as no parity */
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
			
		default:
			return 6;
	}

	/* OPOST--启用具体实现自行定义的输出处理 */
	if(index == 1)
		options.c_oflag &= ~OPOST;			
	
	/*
	ICANON--启用标准模式 (canonical mode)。允许使用特殊字符 EOF, EOL, 
			EOL2, ERASE, KILL, LNEXT, REPRINT, STATUS, 和 WERASE，以及按行的缓冲。 
	ECHO--回显输入字符
	ECHOE--如果同时设置了 ICANON，字符 ERASE 擦除前一个输入字符，WERASE 擦除前一个词
	ISIG--当接受到字符 INTR, QUIT, SUSP, 或 DSUSP 时，产生相应的信号
	*/
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw data

	/* VMIN--非 canonical 模式读的最小字符数 
	   VTIME--非 canonical 模式读时的延时，以十分之一秒为单位
	*/
	options.c_cc[VMIN]	= 0;		// update the options and do it now 
	options.c_cc[VTIME] = 50;		// set timeout 5 seconds
	tcflush(m_fd, TCIFLUSH); /* TCIFLUSH Update the options and do it NOW */
	
	/* TCSANOW--改变立即发生 */
	if (0 != tcsetattr(m_fd, TCSANOW, &options)) 
	{
		return 7;
	}
	
	return 0;
		
}


int CComTransmit::SetComParity(int databits, int stopbits, int parity,int flowctrl)
{
	struct termios options; 
	
	if  ( tcgetattr( m_fd,&options)  !=  0)
	{ 
		printf("SetComParity 1");     
		return-1;
	}

	options.c_cflag &= ~CSIZE;
  	switch (databits) /*设置数据位数*/
  	{
  		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
  		case 7:
  			options.c_cflag |= CS7;
  			break;
  		case 8:
			options.c_cflag |= CS8;
			break;
			
		default:
			fprintf(stderr,"Unsupported data size\n");
			return -1;

	}
	
  	switch (parity)
  	{
  		case 'n':
		case 'N':
		case 0:
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;    /* Disnable parity checking */ 
			break;
		case 'o':
		case 'O':
		case 1:
			options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/ 
			options.c_iflag |= INPCK;             /* Enable parity checking */
			break;
		case 'e':
		case 'E':
		case 2:
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/  
			options.c_iflag |= INPCK;       /* Enable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return -1;
		}
	
 	 /* 设置停止位*/   
  	switch (stopbits)
  	{
  		case 1:
  			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
		break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (-1);
	}

	switch(flowctrl)	/* 设置流控制*/
	{
		case 0:
			options.c_cflag &= ~CRTSCTS;   /*no flow control*/
			break;
		case 1:
			options.c_cflag |= CRTSCTS;    /*hardware flow control*/
			break;
		case 2:
			options.c_cflag |= IXON|IXOFF|IXANY; /*software flow control*/
			break;
		default:
			options.c_cflag &= ~CRTSCTS;   /*no flow control*/
			break;
	}
	
  	/* Set input parity option */
  	if (parity != 'n')
  		options.c_iflag |= INPCK;
	
    	options.c_cc[VTIME] = 150; // 15 seconds
    	options.c_cc[VMIN] = 0;

  	tcflush(m_fd,TCIFLUSH); /* Update the options and do it NOW */
  	if (tcsetattr(m_fd,TCSANOW,&options) != 0)
  	{
  		perror("SetComParity 2");
		return (-1);
	}
	
  	return (0);
	
}

//0---read ,1--write
int CComTransmit::SetComSelectTimeOut(int sec,int usec,SELECT_TIMEOUT_F rwflag)
{
	if(rwflag == WRITE_TIMEOUT_F)
	{
		Wtimeout.tv_sec = sec;
		Wtimeout.tv_usec = usec;
	}
	else
	{
		Rtimeout.tv_sec = sec;
		Rtimeout.tv_usec = usec;
	}
	
	return 0;
}

int CComTransmit::IOFlush()
{
	tcflush(m_fd, TCIOFLUSH); 
	return 0;
}


int CComTransmit::SetComRawMode()
{
	int ret = 0;
	struct termios options; 

	tcgetattr(m_fd, &options);

	tcflush(m_fd, TCIOFLUSH);
//	options.c_cflag &= ~CSIZE;
//	options.c_cflag |= CS8;
//	options.c_cflag &= ~PARENB;
//	options.c_cflag &= ~CSTOPB;
	options.c_oflag  &= ~OPOST;

      	//options.c_iflag |= (IGNBRK|BRKINT);
      	//options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

	//|PARMRK|ISTRIP|IGNCR|ICRNL|IXON|INLCR
	
	tcflush(m_fd,TCIFLUSH); 
	options.c_cc[VTIME] = 128;
	options.c_cc[VMIN] = 1;

	cfmakeraw(&options);

	ret = tcsetattr(m_fd,TCSANOW,&options);
	if (ret < 0) 
	{ 
		perror("set raw mode error!"); 
	} 

	return ret;
}

int CComTransmit::ReadCom(char *ReadBuffer,int size,int *retSize)
{
	fd_set serial;
	int retval;	
	struct timeval rdtimeout;
	if(m_fd < 0)
	{
		fprintf(stderr,"%s[%d]:serial have not open!\n",__FILE__,__LINE__);
		return -1;
	}
    FD_ZERO(&serial);
	FD_SET(m_fd, &serial);
	Rtimeout.tv_sec = 0;
	Rtimeout.tv_usec = 500000;		
	rdtimeout = Rtimeout;
	if ((retval = select(m_fd+1,  &serial, NULL, NULL, &rdtimeout)) <=0)
	{  
		printf("error\n");
		return -1;
	}
	if(FD_ISSET(m_fd,&serial)!=0)
	{

		retval = read(m_fd,  ReadBuffer, 255);
		if(retval > 0)
		{
			*retSize = retval;
		}
		else 
		{
			*retSize = 0;
		}
	}
	return retval;
}

int CComTransmit::WriteCom(char *WriteBuffer,int size)
{
	int     ret = -1;
	fd_set serial; 
       int retval;
	struct timeval WTtimeout;

	if(m_fd < 0)
	{
		fprintf(stderr,"serial have not open!\n");
		return -1;
	}
	
       FD_ZERO(&serial);
       FD_SET(m_fd, &serial);
	WTtimeout = Wtimeout;
       if ((retval = select(m_fd+1,  NULL, &serial, NULL, &WTtimeout)) < 0)
	{		
              fprintf(stderr,"%s[%d]:select error!---------------\n",__FILE__,__LINE__);
		return -1;
       }
	
	if(retval == 0)
	{
		fprintf(stderr,"%s[%d]:select timeout!---------------\n",__FILE__,__LINE__);		
		return -1;
	}
	
	//读数据写BUFFER
       if(FD_ISSET(m_fd,&serial)!=0)
	{
             	ret = write(m_fd, WriteBuffer ,size);
	       if(ret == -1)
		{
	                printf("serial send data failed -----error!\n");
	                return  -1;
	       }
        }
		
        return ret;
}

int CComTransmit::GetPortFD()
{
	return m_fd;
}

FILE* CComTransmit::GetPortFP()
{
	return m_fp;
}


void CComTransmit::ConvertCR2LF(int  Switch)
{
	int ret = 0;
	struct termios   Opt;
	
	tcgetattr(m_fd, &Opt);
	tcflush(m_fd, TCIOFLUSH);
	
    	ret = tcsetattr(m_fd, TCSANOW, &Opt);
    	if(ret != 0)
    	{
            	perror("tcsetattr fd1");
		return;
    	}

	if(Switch)
	{
		Opt.c_oflag  |= OPOST;
	}
	else 
	{
		Opt.c_oflag  &= ~OPOST;
	}

  	tcflush(m_fd,TCIFLUSH); /* Update the options and do it NOW */
  	if (tcsetattr(m_fd,TCSANOW,&Opt) != 0)
  	{
  		perror("Set CR 2 LF  2");
		return;
	}
	
	tcflush(m_fd,TCIOFLUSH);	

}

