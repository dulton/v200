#ifndef _GPIO_LINUX
#define _GPIO_LINUX

#define FALSE 	0
#define TRUE 	1

#define GPIO_FLAG    	_IO('p',0x01)
#define GPIO_SET_DIR 	_IO('p',0x2)
#define GPIO_GET_DIR 	_IO('p',0x3)
#define GPIO_READ_BIT 	_IO('p',0x4)
#define GPIO_WRITE_BIT 	_IO('p',0x5)
#define GPIO_PERIPHCTRL21   _IO('p',0x6)
#define GPIO_WIFI_WPS   _IO('p',0x7) //wps

#define GPIO_LED_CTRL	_IO('p',0x8)

typedef struct {
unsigned int  groupnumber;
unsigned int  bitnumber;
unsigned char  value;
}gpio_groupbit_info;

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
