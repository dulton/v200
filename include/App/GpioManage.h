#ifndef _GPIO_MANAGE_H_
#define _GPIO_MANAGE_H_



#include "GpioApi.h"

bool 	GpioInit();
void 	GpioQuit();
bool 	FeedDog();
bool		ResetADChip();
bool 	ControlLed(bool light);
bool 	ControlPdnPower(bool light);
bool 	ControlCamPower(bool light);

bool 	ControlSpeaker(bool status);
bool 	ResetBluetooth(bool status);
bool 	ResetGPS(bool status);

bool 	GetVMon0status(unsigned int *light);
bool 	GetVMon1status(unsigned int *light);
bool 	GetAlarmInstatus(unsigned int *light);
bool	 	GetAutoOrManualStatus(unsigned int *status);
bool 	GetDefaultKeyStatus(unsigned int *status);


#endif//_GPIO_MANAGE_H_

