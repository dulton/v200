#include <unistd.h>
#include "GpioManage.h"
#include "stdio.h"
#include "stdlib.h"
#include "CommonFunction.h"
#include "CommonDefine.h"

#define BIT_TRUE		1
#define BIT_FALSE	0

//输出方向
#define FEEDDOG_G	1	//gpio1
#define FEEDDOG_B	1	// 1bit 

#define RESET_VIDEO_G		2	//gpio2
#define RESET_VIDEO_B		0	// 0bit

#define CL_LED_G		0	//led
#define CL_LED_B		2

#define CL_PW_CLOSE_G	0//	pdn power manage
#define CL_PW_CLOSE_B	3//

#define CL_CAM_PW_CLOSE_G		3//camera power close
#define CL_CAM_PW_CLOSE_B		5

#define SPK_MUTE_G		0
#define SPK_MUTE_B		1

#define BT_RESET_G		6
#define BT_RESET_B		3

#define RST_GPS_G		0
#define RST_GPS_B		4

#define SD_EN_G			0
#define SD_EN_B			2

//输入方向
#define VMON0_G		0		//power monitor 0
#define VMON0_B		0

#define VMON1_G		0		//power monitor 1
#define VMON1_B		1

#define ALARMIN_G	0		//alram in
#define ALARMIN_B	4

#define DEFAULT_KEY_G	6
#define DEFAULT_KEY_B	0

#define AUTO_MANUAL_G	6
#define AUTO_MANUAL_B	1

void GpioTest()
{
	ControlPdnPower(0);
	ResetADChip();
	
	unsigned int alarm;
	unsigned int powerflag;
	while(1)
	{
		ControlLed(1);
		sleep(1);
		ControlLed(0);
		sleep(1);
		GetAlarmInstatus(&alarm);
		fprintf(stderr, "alarm = %d\n", alarm);

		GetVMon0status(&powerflag);
		fprintf(stderr, "powerflag = %d\n", powerflag);
	}
}

/*
10.	GPIO0_4，输出，定义为RST_GPS。平时应输出1，输出0则为复位GPS模块。
*/

bool GpioInit()
{
	if(HI_SetGpio_Open())
	{
		return true;
	}

	//初始化输出方向
	Hi_SetGpio_SetDir(FEEDDOG_G,FEEDDOG_B,GPIO_DIR_OUT);
	Hi_SetGpio_SetDir(RESET_VIDEO_G,RESET_VIDEO_B, GPIO_DIR_OUT);

	Hi_SetGpio_SetDir(CL_PW_CLOSE_G, CL_PW_CLOSE_B, GPIO_DIR_OUT);
	Hi_SetGpio_SetDir(CL_CAM_PW_CLOSE_G, CL_CAM_PW_CLOSE_B, GPIO_DIR_OUT);

#ifdef Y_BOARD
	//Reset GPS
	Hi_SetGpio_SetDir(RST_GPS_G, RST_GPS_B, GPIO_DIR_OUT);
	Hi_SetGpio_SetBit(RST_GPS_G, RST_GPS_B, 1);
	
	//bluetooth reset
	Hi_SetGpio_SetDir(BT_RESET_G, BT_RESET_B, GPIO_DIR_OUT);
	Hi_SetGpio_SetBit(BT_RESET_G, BT_RESET_B, 0);
//	usleep(100000);		//modify to below by dada 2009-02-04
	usleep(100);
	Hi_SetGpio_SetBit(BT_RESET_G, BT_RESET_B, 1);

	//spk_mute
	Hi_SetGpio_SetDir(SPK_MUTE_G, SPK_MUTE_B, GPIO_DIR_OUT);
	Hi_SetGpio_SetBit(SPK_MUTE_G, SPK_MUTE_B, 1);

	//恢复gps 模块工作
	//Hi_SetGpio_SetBit(RST_GPS_G, RST_GPS_B, 1);

	//sd_enable
	//Hi_SetGpio_SetDir(SD_EN_G, SD_EN_B, GPIO_DIR_OUT);
	//Hi_SetGpio_SetBit(SD_EN_G, SD_EN_B, 0);
#else
	Hi_SetGpio_SetDir(CL_LED_G, CL_LED_B, GPIO_DIR_OUT);
	ControlLed(1);
#endif

	//GpioTest();
	
	//初始化输入方向
	Hi_SetGpio_SetDir(VMON0_G,VMON0_B,GPIO_DIR_IN);

#ifdef Y_BOARD
	//Default para set
	Hi_SetGpio_SetDir(DEFAULT_KEY_G, DEFAULT_KEY_B, GPIO_DIR_IN);

	//AUTO /MANUAL
	Hi_SetGpio_SetDir(AUTO_MANUAL_G, AUTO_MANUAL_B, GPIO_DIR_IN);
#else
	Hi_SetGpio_SetDir(VMON1_G, VMON1_B, GPIO_DIR_IN);
	Hi_SetGpio_SetDir(ALARMIN_G, ALARMIN_B,GPIO_DIR_IN);
#endif

#ifdef Y_BOARD
	
#else
#endif
	
	return true;
}

void GpioQuit()
{
	Hi_SetGpio_Close();
}

bool FeedDog()
{
	static bool lg = true;	
	lg = !lg;	
	if(Hi_SetGpio_SetBit(FEEDDOG_G,FEEDDOG_B, lg) < 0)
	{
		printf("freed dog is failure!\n");
		return false;
	}
	return true;
}

bool ResetADChip()
{
	if(Hi_SetGpio_SetBit(RESET_VIDEO_G, RESET_VIDEO_B, BIT_FALSE) < 0)
	{
		printf("ResetADChip failure!\n");
		return false;
	}

	return true;
}

bool ControlLed(bool light)
{
#ifndef Y_BOARD
	if(Hi_SetGpio_SetBit(CL_LED_G, CL_LED_B, light) < 0)
	{
		printf("ControlLed failure!\n");
		return false;
	}
#endif
	return true;
}

bool ControlSpeaker(bool status)
{
#ifdef Y_BOARD
	if(Hi_SetGpio_SetBit(SPK_MUTE_G, SPK_MUTE_B, status) < 0)
	{
		printf("ControlSpeaker failure!\n");
		return false;
	}
#endif
	return true;
}

bool ResetBluetooth(bool status)
{
#ifdef Y_BOARD
	if(Hi_SetGpio_SetBit(BT_RESET_G, BT_RESET_B, status) < 0)
	{
		printf("ResetBluetooth failure!\n");
		return false;
	}
#endif
	return true;
}

bool ResetGPS(bool status)
{
#ifdef Y_BOARD
	Hi_SetGpio_SetBit(RST_GPS_G, RST_GPS_B, 0);
	sleep(1);
	Hi_SetGpio_SetBit(RST_GPS_G, RST_GPS_B, 1);
	
/*	if(Hi_SetGpio_SetBit(RST_GPS_G, RST_GPS_B, status) < 0)
	{
		printf("ResetMcu failure!\n");
		return false;
	}*/
#endif
	return true;
}


bool ControlPdnPower(bool light)
{
	if(Hi_SetGpio_SetBit(CL_PW_CLOSE_G, CL_PW_CLOSE_B, light) < 0)
	{
		printf("ControlPdnPower failure!\n");
		return false;
	}

	return true;
}

bool ControlCamPower(bool light)
{
	if(Hi_SetGpio_SetBit(CL_CAM_PW_CLOSE_G, CL_CAM_PW_CLOSE_B, light) < 0)
	{
		printf("ControlCamPower failure!\n");
		return false;
	}

	return true;
}

bool GetVMon0status(unsigned int *light)
{
	if(Hi_SetGpio_GetBit(VMON0_G,VMON0_B,light) <0)
	{
		printf("GetVMon0status failure!\n");
		return false;
	}
	return true;
}

bool GetVMon1status(unsigned int  *light)
{
	if(Hi_SetGpio_GetBit(VMON1_G,VMON1_B,light) <0)
	{
		printf("GetVMon1status failure!\n");
		return false;
	}
	return true;
}

bool GetAlarmInstatus(unsigned int *light)
{
	if(Hi_SetGpio_GetBit(ALARMIN_G,ALARMIN_B,light) <0)
	{
		printf("GetAlarmInstatus failure!\n");
		return false;
	}
	return true;
}

bool GetAutoOrManualStatus(unsigned int *status)
{
	if(Hi_SetGpio_GetBit(AUTO_MANUAL_G ,AUTO_MANUAL_B, status) <0)
	{
		printf("GetAutoOrManualStatus failure!\n");
		return false;
	}
	return true;
}

bool GetDefaultKeyStatus(unsigned int *status)
{
	if(Hi_SetGpio_GetBit(DEFAULT_KEY_G ,DEFAULT_KEY_B, status) < 0)
	{
		printf("get default key failure!\n");
		return false;
	}
	return true;
}

