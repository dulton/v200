#ifndef _GPIO_LINUX
#define _GPIO_LINUX

#define FALSE 	0
#define TRUE 	1

#define GPIO_FLAG    		_IO('p',0x01)//复位IO
#define GPIO_SET_DIR 		_IO('p',0x2)
#define GPIO_GET_DIR 		_IO('p',0x3)
#define GPIO_READ_BIT 		_IO('p',0x4)
#define GPIO_WRITE_BIT 		_IO('p',0x5)
#define GPIO_PERIPHCTRL21   _IO('p',0x6)
#define GPIO_WIFI_WPS 		_IO('p',0x7)//WPS IO
//#define GPIO_WIFI_WPS 		_IO('p',0x01) //复位键和WPS按键复用IO

#define GPIO_LED_CTRL		_IO('p',0x8)
#define GPIO_ADUIO_AMPLIFIER	_IO('p',0x9)


enum LED{
	LED_R = 0,
	LED_B,
	LED_G
};

struct LED_STATUS{
	enum LED led;
	bool on;
};

#endif
