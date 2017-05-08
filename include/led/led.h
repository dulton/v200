#ifndef LED_H
#define LED_H


void OpenLedGpio();
bool WIFI_LED_ON(int Color);
bool WIFI_LED_OFF(int Color);
void InitStartLed();
void ConnectLed();
void CreateWifiLedProcess(void *status);
void SetSysRestLED();


#endif 
