#ifndef _IRCUT_H
#define _IRCUT_H

#define IRCUT_FLAG    _IO('p', 0x01)
#define IRCUT_ADMAX    _IO('p', 0x02)
#define IRCUT_ADMIN    _IO('p', 0x03)
#define IRCUT_ADFILP    _IO('p', 0x04)
#define IRCUT_NIGHTSWITCH  _IO('p', 0x05)


int  Init_IRCut(void);

int  Config_IRCutAD(unsigned int max,unsigned int min,unsigned char filp);

int  Get_IRCutStatus(unsigned char &status);

int  Set_NightSwtich(unsigned char status);

#endif
