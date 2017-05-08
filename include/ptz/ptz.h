
#ifndef __PTZ_H__ 
#define __PTZ_H__

#include <pthread.h>
#include <errno.h>
#include <string.h>
	   
#include "common.h"
#include "parametermanage.h"
#include "ComTransmit.h"


#define PELCO_P_CODE1		0x1f
#define PELCO_P_CODE2		0x1f

#define PTZ_READ		0
#define PTZ_WRITE		1



typedef enum 
{
	CMD_STOP = 0,
	CMD_LEFT,
	CMD_RIGHT,
	CMD_UP,
	CMD_DOWN,
	 
	CMD_CALL_CRIUSE = 0x12, // 呼叫巡航
	 
	CMD_AUTOSCAN = 0x13,
	 
	CMD_CALLPRESET = 0x15,
	CMD_CALL_KINDSCAN = 0x16,  //呼叫花样扫描
	 
	CMD_FOCUSFAR = 0x23,
	CMD_FOCUSNAER = 0x24,
	CMD_IRISOPEN = 0x25,
	CMD_IRISCLOSE = 0x26,
	CMD_ZOOMTELE = 0x27,
	CMD_ZOOMWIDE  = 0x28,
	 
	/*CMD_SET_CRIUSE_P = 0x32, //设置巡航点*/
	CMD_SETPRESET = 0x35,
	CMD_CRIUSE = 0x36,
	CMD_CLRPRESET = 0x37,
	CMD_STOPSCAN = 0x38,
	 
	CMD_SET_DWELLTIME=0x39,
	 
	CMD_KINDSCAN_START= 0x3A,
	 
	CMD_KINDSCAN_END= 0x3B,
	 
	CMD_CLRCRIUES_LINE= 0x3C,
	 
	CMD_CLR_SCAN_LINE = 0x3D,
	CMD_CLR_KINDSCAN= 0x3E,
	 
	 
	CMD_AUTOSCANSPEED = 0x3F,
	 
	CMD_SET_PRESET = 0x40,
	 
	CMD_CAL_PRESET = 0x41,
	 
	CMD_GET_PRESET = 0x42,
	 
	CMD_SET_RESET = 0x43, /*恢复出厂值*/
	 
	CMD_V_SCAN = 0x44,
	CMD_DEL_PRESET = 0x45,
}PTZ_CMD_E;


typedef enum
{
	NULL_PROTOCOL = 0,
	PELCO_D_PRTCL,	
	PELCO_P_PRTCL,
	#if 1	//zlz add ptz protocol
	//排列顺序要和DVR云台设置界面的下拉框一致
	SAMSUNG_PRTCL,
	NEOCAM_PRTCL,	
	LILIN1016_PRTCL,	
	LILIN_FASTDOME_PRTCL,	
	SAE_PRTCL,		
	RM110_PRTCL,
	#endif
	
}PTZ_PROTOCOL_E;

int InitPTZ(unsigned int speed);
int PTZ_Operate(int s32Chn,PTZ_CMD_E enCmd, int  s32Spd,int other);
int GetPTZResetPoint(char *buffer,int *len);


int InitMotorPT();
int UnInitMotorPT();
int MotorPT_Operate(PTZ_CMD_E enCmd, int  s32Spd,int other);
int MotorTurnAroundTest(int flag);

void AutoScan(bool sustain);

int Motor_Ctrl();	
	

#endif 

