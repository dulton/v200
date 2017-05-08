
#ifndef __HI_IR_API_H__
#define __HI_IR_API_H__

#define IR_IOC_SET_BUF               0x01


#define IRDEVNAME 				"/dev/Hi_IR"


#define  IR_ShutDown				0x0c
#define  IR_VGA					0x0f
#define  IR_LEFT					0x03
#define  IR_Right					0x02
#define  IR_Up					0x00
#define  IR_Down					0x01
#define  IR_Enter					0x1f
#define  IR_Back					0x06
#define  IR_Lock					0x0a
#define  IR_0						0x10
#define  IR_1						0x11
#define  IR_2						0x12
#define  IR_3						0x13
#define  IR_4						0x14
#define  IR_5						0x15
#define  IR_6						0x16
#define  IR_7						0x17
#define  IR_8						0x18
#define  IR_9						0x19
#define  IR_Delete				0x0d
#define  IR_DubleNum			0x1a
#define  IR_View					0x0e
#define  IR_Manual_Rec			0x48
#define  IR_Play					0x58
#define  IR_PlayMode				0x44
#define  IR_PlayBefore			0x50
#define  IR_PlayNext				0x54
#define  IR_PlayBack				0x1e
#define  IR_PlayFast				0x4c
#define  IR_ZOOM_INC			0x04
#define  IR_ZOOM_DEC			0x08
#define  IR_FOCUS_INC			0x1c
#define  IR_FOCUS_DEC			0x1d
#define  IR_Alarm_Arm			0x40
#define  IR_Alarm_DisArm		0x05
#define  IR_PSET					0x1b
#define  IR_SCAN					0x5c
#define  IR_PATT					0x0b
#define  IR_GROUP				0x09
#define  IR_MENU					0x07			

#define  IR_Reserved1					0x41	
#define  IR_Reserved2					0x45	
#define  IR_Reserved3					0x49	
#define  IR_Reserved4					0x51	



/* DEFINE KEY STATE */
#define HIIR_KEY_DOWN 0x00
#define HIIR_KEY_UP   0x01

typedef struct
{
    unsigned long irkey_datah;
    unsigned long irkey_datal;
    unsigned long irkey_state_code;
}irkey_info_s;

int HandleIrCode(int IrCode);

void IRStart();

int IR_Read(unsigned int  *IrCode);



#endif 

