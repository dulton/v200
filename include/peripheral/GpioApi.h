#ifndef GPIO_API_H
#define GPIO_API_H

#define GPIO_DIR_IN 	0
#define GPIO_DIR_OUT 	1

int  HI_SetGpio_Open(void);
int  Hi_SetGpio_Close(void);

int  Hi_SetGpio_SetDir(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned gpio_dir );
int  Hi_SetGpio_GetDir(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned* gpio_dir );

int  Hi_SetGpio_SetBit(unsigned int gpio_groupnum,unsigned int gpio_bitnum, unsigned int bitvalue);
int  Hi_SetGpio_GetBit(unsigned int gpio_groupnum,unsigned int gpio_bitnum,unsigned int* bitvalue);

int HiGetResetGpio(unsigned char *value);

int  HiGpioInit();


typedef struct {
unsigned int  groupnumber;
unsigned int  bitnumber;
unsigned int  value;
}gpio_groupbit_info;

#define GPIO_FLAG    	_IO('p',0x01)
#define GPIO_SET_DIR 	_IO('p',0x2)
#define GPIO_GET_DIR 	_IO('p',0x3)
#define GPIO_READ_BIT 	_IO('p',0x4)
#define GPIO_WRITE_BIT 	_IO('p',0x5)

#define GPIO_SET_BIT		GPIO_WRITE_BIT
#define GPIO_GET_BIT		GPIO_READ_BIT
#define GPIO_GET_BIT_DIR	GPIO_GET_DIR
#define GPIO_SET_BIT_DIR	GPIO_SET_DIR

/*

#define GPIO_SET_DIR 		0x2
#define GPIO_GET_DIR 		0x3
#define GPIO_READ_BIT 		0x4
#define GPIO_WRITE_BIT 		0x5


#define Magic_GPIO            'G'

#define GPIO_SET_BIT		_IOW(Magic_GPIO,0,unsigned char)
#define GPIO_GET_BIT		_IOR(Magic_GPIO,1,unsigned char)

#define GPIO_GET_BIT_DIR	_IOR(Magic_GPIO,2,unsigned char)
#define GPIO_SET_BIT_DIR	_IOW(Magic_GPIO,3,unsigned char)
#define GPIO_MODE_SET		_IOW(Magic_GPIO,4,unsigned char)
#define GPIO_REMAP			_IOW(Magic_GPIO,5,unsigned char)
#define GPIO_UNMAP			_IOW(Magic_GPIO,6,unsigned char)
*/

#endif
