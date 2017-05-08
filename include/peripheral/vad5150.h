#ifndef __VAD5150_H__
#define __VAD5150_H__

#include "common.h"

 #define I2C_TVP5150               0xBA
 #define I2C_TVP5150_2		0xB8

 #define Magic_TVP5150            'G'

 #define WRITE_VAD5150_REGISTER 	_IOW(Magic_TVP5150,0,unsigned short)

#define  READ_VAD5150_REGISTER		_IOR(Magic_TVP5150,1,unsigned short)

 #define VIDEO_MODE_PAL 			_IOW(Magic_TVP5150,2,unsigned short)

#define  VIDEO_MODE_NTSC			_IOR(Magic_TVP5150,3,unsigned short)

#define   VAD_BRIGHTNESS_SET		0	

#define	VAD_CONTRAST_SET			1

#define	VAD_SATURATE_SET			2

#define	VAD_HUE_SET				3

#define   VAD_BRIGHTNESS_GET		4	

#define	VAD_CONTRAST_GET			5

#define	VAD_SATURATE_GET			6

#define	VAD_HUE_GET				7

class HWVAD5150
{

private:
	
	int 	m_n32Fd;
	
	static HWVAD5150   *m_pInstance;
	
public:

	HWVAD5150();
	
	~HWVAD5150();

	static HWVAD5150*  Instance();
	
	int Set5150Normal(int mode);

	int GetVideoStatus();

	int WriteVad5150Register(int type,int value);

	int ReadVad5150Register(int type,int *value);
	
};


#endif 

