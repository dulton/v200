#ifndef CSPARA_H_
#define CSPARA_H_
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef DM368
#define DM36XCHANN		(3)					
typedef short   Bool;                
typedef struct _OSDPrm 					
{
    int detailedInfo;
    int	dateEnable;
    int	timeEnable;
    int	logoEnable;
    int logoPos;
    int	textEnable;
    int textPos;
    char text[17];						///<max len 17. org:24.
} OSDPrm;
#endif
int csSetNetWorkPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetSecurityPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetMachinePara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCameraPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCameraVideoCodePara(CAMERA_PARA *plocalPara, CAMERA_PARA *pcsPara);
int csSetAnalogPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetMPowerPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetComParaPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCamBlindPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCamMdPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetRecSchedPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetPcDirPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetAlarmInPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetMainetancePara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetDisplayPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetUsersPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetSysExceptPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetOsdInsertPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetPTZPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetPELCOPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetPicTimerPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetMegaPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetZoneGroupPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetDefSchedPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetPtzLinkPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetMainPanelPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSet3GPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetAutoSwitchPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCamVideoLossPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

int csSetCamAlarmPortPara(SYSTEM_PARAMETER *plocalPara, SYSTEM_PARAMETER *pcsPara);

#endif
