#ifndef _GPIO_MOTOR
#define _GPIO_MOTOR

#define FALSE 	0
#define TRUE 	1

#define MOTOR_CTRL    		_IO('p',0x01)

enum  MOTOR_TYPE {
	MOTOR_H = 0x00,
	MOTOR_V = 0x01,
	MOTOR_TYPE_OTHER
};

enum MOTOR_DIRECT {
	REVERSE = 0x00,
	FORWORD = 0x01,
	MOTOR_DIRECT_OTHER
};

typedef struct {
enum MOTOR_TYPE   		type; 
enum MOTOR_DIRECT   	direct;
unsigned char   		speed; /*1-255*/

/*
	==限位开关的状态==
  bit:0 1,水平电机到最左
  bit:1 1,水平电机到最右
  bit:2 1,垂直电机到最下
  bit:3 1,垂直电机到最上
*/
unsigned char           status;
}motor_info;


#endif
