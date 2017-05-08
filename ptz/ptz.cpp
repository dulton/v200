
#include     <stdio.h>      /*标准输入输出定义*/
#include     <stdlib.h>     /*标准函数库定义*/
#include     <unistd.h>     /*Unix标准函数定义*/
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX终端控制定义*/

#include "ptz.h"
#include "ModuleFuncInterface.h"
#include "interfacedef.h"
#define  ptz_debug
#define SERIALDEVICE		"/dev/ttyAMA1"
int ptz_fd = -1;
#ifdef MINIPT_IPC	
#define PRESET_LENGTH	130
#else
#define PRESET_LENGTH	195
#endif

char 	Resetpoint[PRESET_LENGTH] ={0x0};
bool	GetResepoint = false;
int SetComSpeed(unsigned int baud_rate)
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
		if(name_arr[i]== baud_rate)
			break;
	}

	if(i >= sizeof(name_arr)/sizeof(int))
		return 1;

	//if(fd == rs232_fd)
	//	index = 0;
	//else
		index = 1;

	memset(&options, 0x00, sizeof(options));
	if (0 != tcgetattr(ptz_fd, &options)) 
	{
		return 3;
	}

	tcflush(ptz_fd, TCIOFLUSH); //清空缓存
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
	tcflush(ptz_fd, TCIFLUSH); /* TCIFLUSH Update the options and do it NOW */
	
	/* TCSANOW--改变立即发生 */
	if (0 != tcsetattr(ptz_fd, TCSANOW, &options)) 
	{
		return 7;
	}
	
	return 0;
		
}

int InitPTZ(unsigned int speed)
{
#ifdef WIFI_18E_IPC
	InitMotorPT();
    return 0;
#else
	ptz_fd = open(SERIALDEVICE,O_RDWR);
	if(ptz_fd < 0)
	{
		printf("Open Com [%s] failure!\n",SERIALDEVICE);
		return -1;
	}
	memset(Resetpoint,0x30,PRESET_LENGTH);
	return SetComSpeed(speed);
#endif
}
/*
wr =0 :read
wr =1 :write
*/
int SerialOperate(char wr,char *buf,int len)
{
	
	fd_set fds;	
	int retval = 0;
	struct timeval timeoutval; 
	FD_ZERO(&fds);
	FD_SET(ptz_fd, &fds);
	
	timeoutval.tv_sec = 2;
	timeoutval.tv_usec = 0;
	
	if(wr == 0)
	{	


		
		retval = select(ptz_fd+1, &fds, NULL, NULL, &timeoutval);
		if(retval > 0)
		{	
			
			if(FD_ISSET(ptz_fd, &fds))
			{	

				retval = read(ptz_fd, buf, len);				
				printf("###################:%d\n",retval);
			}
		
		}
		else
		{
			return -1;
		}
		
	}
	else
	{
		retval = select(ptz_fd+1,  NULL, &fds,NULL, &timeoutval);
		if(retval > 0)
		{	
			if(FD_ISSET(ptz_fd, &fds))
			{	
				retval = write(ptz_fd, buf, len);		
				
			}		
		}
		else
		{
			return -1;
		}

	}
	
	return retval;
  
}




int PTZ_Operate(int s32Chn,PTZ_CMD_E enCmd, int  s32Spd,int other)
{
#ifdef WIFI_18E_IPC
	printf("PTZ_Operate ptz_fd:%d ,enCmd:%d ,s32Spd:%d\n",ptz_fd,enCmd,s32Spd);

	MotorPT_Operate(enCmd,s32Spd,other);
    return 0;
#else
	char buff[32];
	int  nwrite = 0;
	int  valule =0;

	
	memset(buff,0,32);
	
	switch (enCmd)
	{
	 case CMD_STOP:
		buff[0] = 0xff;
		buff[1] = s32Chn;
		buff[2] = 0x00;
		buff[3] = 0x00;
		buff[4] = 0x00;
		buff[5] = 0x00; //*ff  1  0  0	0  0  1
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
		break;
	 case CMD_LEFT:
		buff[0] = 0xFF;
		buff[1] = s32Chn;
		buff[2] = 0x00;
		buff[3] = 0x04;
		buff[4] = s32Spd;
		buff[5] = 0x00;
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
#ifdef MINIPT_IPC
		Resetpoint[129] =s32Spd;	
#else
#endif
	 	break;	 
	 case CMD_RIGHT:
		buff[0] = 0xFF;
		buff[1] = s32Chn;
		buff[2] = 0x00;
		buff[3] = 0x02;
		buff[4] = s32Spd;
		buff[5] = 0x00;
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
#ifdef MINIPT_IPC
		Resetpoint[129] =s32Spd;	
#else
#endif

	 	break;
	 
	 case CMD_UP:
		buff[0] = 0xFF;
		buff[1] = s32Chn;
		buff[2] = 0x00;
		buff[3] = 0x08;
		buff[4] = 0x00;
		buff[5] = s32Spd;
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
#ifdef MINIPT_IPC
		Resetpoint[129] =s32Spd;	
#else
#endif

		 break;
	 
	 case CMD_DOWN:
		buff[0] = 0xFF;
    	buff[1] = s32Chn;
		buff[2] = 0x00;
		buff[3] = 0x10;
		buff[4] = 0x00;
		buff[5] = s32Spd;
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
#ifdef MINIPT_IPC
		Resetpoint[129] =s32Spd;	
#else
#endif

	 	break;
	 case CMD_AUTOSCANSPEED:
	 	buff[0] = 0xFF;
		buff[1] = 0x01;
		buff[2] = 0x00;
		buff[3] = 0x03;
		buff[4] = 0x00;
		buff[5]=0x40;
		if((s32Spd<16)&&(s32Spd>=0))
		{
			buff[5]=0x3d;
		}
		if((s32Spd<32)&&(s32Spd>=16))
		{
			buff[5]=0x3e;
		}
		if((s32Spd<48)&&(s32Spd>=32))
		{
			buff[5]=0x3f;
		}
		if((s32Spd<64)&&(s32Spd>=48))
		{
			buff[5]=0x40;
		} 
		
		//buff[5] = s32Spd;
		buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
		//printf("%x,%x,%x,%x,%x,%x,%x\n",buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6]);
		break;

	 case CMD_AUTOSCAN:  //FF 01 00 07 00 26 2E
		 buff[0] = 0xFF;
		 buff[1] = 0x01;;
		 buff[2] = 0x00; // 0x00;
		 buff[3] = 0x07; // 0x07;
		 buff[4] = s32Spd; // 0x00;
		 buff[5] = 0x26; // 0x26;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
		 break;
	case CMD_SET_PRESET:  
			buff[0] = 0xFF;
	    	buff[1] = s32Chn;
			buff[2] = 0x00;
			buff[3] = 0x03;
			buff[4] = s32Spd;
			buff[5] = 0x0;
			buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
			#ifdef MINIPT_IPC
			valule = s32Spd -1;
			#else
			valule = s32Spd;
			#endif
			if((valule>=0)&&(valule<PRESET_LENGTH))
			{
				Resetpoint[valule] =0x31;
				printf("--------set------------reset point:%d\n",s32Spd);
			}
			else
			{
				printf("--------error------------reset point:%d\n",valule);
			}
			usleep(200000);
			break;	

	 case CMD_SET_DWELLTIME:
		 buff[0] = 0xFF;
		 buff[1] = s32Chn;
		 buff[2] = 0x00;
		 buff[3] = 0x03;
		 buff[4] = 0x00;
		 buff[5] = s32Spd;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
#ifdef MINIPT_IPC
		Resetpoint[128] =other;	 
#else
#endif

		 break;

	 case CMD_CAL_PRESET:
		 buff[0] = 0xFF;
		 buff[1] = s32Chn;
		 buff[2] = 0x00;
		 buff[3] = 0x07;
		 buff[4] = s32Spd;
		 buff[5] = 0x0;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;

		 break;

	 case CMD_CLRCRIUES_LINE:
		 buff[0] = 0xFF;
		 buff[1] = s32Chn;
		 buff[2] = 0x00;
		 buff[3] = 0x07;
		 buff[4] = other;
		 buff[5] = s32Spd;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;

		 break;

	 case CMD_GET_PRESET:
		 buff[0] = 0xFF;
		 buff[1] = s32Chn;
		 buff[2] = 0x01;
		 buff[3] = 0x01;
		 buff[4] = 0x01;
		 buff[5] = 0x01;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;

		 break;		 
	 case CMD_SET_RESET:
		 buff[0] = 0xFF;
		 buff[1] = s32Chn;
		 buff[2] = 0x00;
		 buff[3] = 0x03;
		 buff[4] = 0x00;
		 buff[5] = 0x28;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;
	 
		 break;
	 case CMD_V_SCAN:
		 buff[0] = 0xFF;
		 buff[1] = 0x1;
		 buff[2] = 0x00;
		 buff[3] = 0x07;
		 buff[4] = s32Spd;
		 buff[5] = 0x27;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100;	 
		 break;
	case CMD_DEL_PRESET:
		 buff[0] = 0xFF;
		 buff[1] = 0x01;
		 buff[2] = 0x00;
		 buff[3] = 0x05;
		 buff[4] = 0x00;
		 buff[5] = s32Spd;
		 buff[6] = (buff[1]+buff[2]+buff[3]+buff[4]+buff[5])%0x100; 
#ifdef MINIPT_IPC
		 valule = s32Spd -1;
#else
		 valule = s32Spd;
#endif

		 if((valule>=0)&&(valule<PRESET_LENGTH))
		 {
			 Resetpoint[valule] =0x30;
			 printf("--------del------------reset point:%d\n",s32Spd);
		 }
		 else
		 {
			 printf("--------error------------reset point:%d\n",valule);
		 }
		 usleep(200000);
		 break;

	 default:
		printf("Wrong Command");
      	 	return -1;

	}
	buff[7] = 0x0d;
	buff[8] = 0x0a;

	if(ptz_fd != -1)
	{
		nwrite = SerialOperate(PTZ_WRITE,buff,9);
		if(nwrite >0 )
		{
			printf("PTZ_Operate write :");
			for(int i=0;i<nwrite;i++)
			{
				printf("%02x ",buff[i]);
			}
			printf("\n");

		}
		else
		{
			printf("SerialOperate failed :%d\n",nwrite);
		}

	}

	return nwrite;
#endif
}

int GetPTZResetPoint(char *buffer,int *len)
{
#ifdef WIFI_18E_IPC

    return 0;
#else
	printf("\n--------------------------------GetPTZResetPoint\n");
	if((buffer ==NULL)&&(len==NULL))
	{
		return -1;
	}	
	int ret =   0;
	int getresult = -1;
	int readpos =0;
	int getcnt=0;
	if(GetResepoint)
	{
		memcpy(buffer,Resetpoint,PRESET_LENGTH);
		*len = PRESET_LENGTH;
		getresult = 0;
	}
	else
	{
		char getPooint[]={0xff,0x01,0x01,0x01,0x01,0x01,0x01,0x0d,0x0a};
		if(SerialOperate(PTZ_WRITE,getPooint,9)==-1)
		{
			return -1;
		}
		sleep(1);
		while(ret!=PRESET_LENGTH)
		{

			
			ret+=SerialOperate(PTZ_READ,Resetpoint+readpos,255);
			readpos = ret;
			if(getcnt++>5)
				break;
		}
		*len = ret;
		if(ret==PRESET_LENGTH)
		{
			GetResepoint = true;
			memcpy(buffer,Resetpoint,PRESET_LENGTH);
			getresult = 0;
		}
	}
	
	printf("\n--------------------------------\n");
	for(int i=0;i<*len;i++)
	{
		printf("%02x ",buffer[i]);
		if(i%10==0)
			printf("\n");
	}
	printf("\n--------------------------------getresult:%d\n",getresult);
	return getresult;
#endif
}



