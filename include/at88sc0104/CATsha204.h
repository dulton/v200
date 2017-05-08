/*
* Copyright(C), 2011-2012 // 版权声明
* File name: //CATsha204.h
* Author:     // 谢成林 LukeXie
* Version:   // V1.0
* Date: // 2012-11-20
* Description: // 把ATSHA204加密IC的功能进行封装，方便使用头文件
*/






#ifndef __ATSHA204_H__ 
#define __ATSHA204_H__
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sha256.h"

#include "GpioApi.h"


#define CONFIG_ZONE 0X00
#define OPT_ZONE 0X01
#define DATA_ZONE 0X02
#define BYTE4 0x04
#define BYTE32 0x20
#define WRITE 0xfe
#define READ 0x01



//读写ATSHA204时返回的错误代码表
typedef enum 
{
    SUCCESS=0x00,                  //命令执行成功
	MAC_DO_NOT_MATCH=0x01,         // MAC不匹配
	PARSE_ERROR=0x03,              //解析错误
	EXEC_ERROR=0x0f,               //执行错误
	WAKE_ERROR=0x11,               //唤醒后，但在此之前的第一个命令
	CRC_OR_OTHER_ERROR=0xff        //CRC或通信错误
}ErrorCodeTable;



//ATSHA204的字地址
typedef enum 
{
	SHA204_I2C_PACKET_FUNCTION_RESET,  //!< Reset device.
	SHA204_I2C_PACKET_FUNCTION_SLEEP,  //!< Put device into Sleep mode.
	SHA204_I2C_PACKET_FUNCTION_IDLE,   //!< Put device into Idle mode.
	SHA204_I2C_PACKET_FUNCTION_NORMAL  //!< Write / evaluate data that follow this word address byte.
}I2cWordAddr;



//ATSHA204的命令集
typedef enum  
{
    PAUSE_CMD=0x01,       //Selectively put just one chip on a shared bus into the idle state
    READ_CMD=0x02,        //Read four bytes from the chip, with or without authentication and encryption
    MAC_CMD=0x08,         //Calculate response from key and other internal data using SHA-256
    HMAC_CMD=0x11,        //Calculate response from key and other internal data using HMAC/SHA-256 
    WRITE_CMD=0x12,       //Write 4 or 32 bytes to the chip, with or without authentication and encryption
    GENDIG_CMD=0x15,      //Generate a data protection digest from a random or input seed and a key
    NONCE_CMD=0x16,       //Generate a 32-byte random number and an internally stored nonce
    LOCK_CMD=0x17,        //Prevent further modifications to a zone of the chip
    TEMP_SENSE_CMD=0x18,  //Return current reading from the optional temperature sensor
    RENDOM_CMD=0x1b,      //Generate a random number
    DERIVE_KEY_CMD=0x1c,  //Derive a target key value from the target or parent key
    UPDATA_EXTRA_CMD=0x20,//Update bytes 84 or 85 within the configuration zone after the configuration zone is locked
    CHECK_MAC_CMD=0x28,   //Verify a MAC calculated on another Atmel CryptoAuthentication device
    DEV_REV_CMD=0x30      //Return device revision information  
}ATsha204Cmd;

#define SC_CLK		0x0506			/*gpio5_6  SCL*/
#define SC_SDA		0x0507			/* gpio5_7  SDA*/

#define 		GPIO_OUTPUT		1
#define 		GPIO_INPUT			0  

       


void SetSclDir(char dir);
void SetSclDat(char dat);



void SetSdaDir(char dir);
void SetSdaDat(char dat);


unsigned char GetSdaDat(void);

void Delay_us(unsigned long us)	;


void Delay_ms(unsigned long ms);


void IoInit(void);

int CheckATsha204();




//ATSHA204的操作类
class CATsha204
{
private:
	unsigned char TagAddr;	
	unsigned char *pSlot[16];
	unsigned char ID[9];

public:
	/*
	* Function:     // 类的构造函数
	* Called By:    // 
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 无
	*/
	CATsha204();



	/*
	* Function:     // 类的析构函数
	* Called By:    // 
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 无
	*/
	~CATsha204();




	/*
	* Function:     // Write命令函数
	* Called By:    // 
	* Input:        // zone，要写哪个区取值为CONFIG_ZONE,OPT_ZONE,DATA_ZONE  (0,1,2)
					// byteSize 要写入的字节数，取值为 4或32  BYTE4 , BYTE32
					// addr 要写入的起始地址
					// buf 要写入的数据 4或32字节 长度是byteSize
	* Output:       // 无
	* Return:       // 1成功。0失败
	* Others:       // 无
	*/
	unsigned char Write(unsigned char zone,unsigned char byteSize,unsigned short addr,unsigned char *buf);






	/*
	* Function:     // read命令函数
	* Called By:    // 
	* Input:        // zone要读哪个区取值为CONFIG_ZONE,OPT_ZONE,DATA_ZONE  (0,1,2)
					// byteSize 要读出的字节数，取值为 4或32  BYTE4 , BYTE32
					// addr 要读出的起始地址
	* Output:       // RecBuf 读出的数据，长度为3+byteSize RecBuf[0]为包长，最后两字节为CRC16，中间为读出数据
	* Return:       // 1成功，0失败
	* Others:       // 无
	*/
	unsigned char read(unsigned char zone,unsigned char byteSize,unsigned short addr,unsigned char *RecBuf);





	/*
	* Function:     // 密码验证
	* Called By:    // 
	* Input:        // slotNum 选择第几个密码进行验证，0-15个密码，
					// RandomNum 32字节的随机数
	* Output:       // recBuf 存放返回的结果，共35字节
	* Return:       // 0 失败，1成功
	* Others:       // recBuf[0]为接收数据总长度，最后两个字节为校验码CRC16，中间的32个数为计算结果			
	*/
	unsigned char Mac(unsigned char slotNum,unsigned char *RandomNum,unsigned char *recBuf);




	
	/*
	* Function:     // 配置区被锁后，可以更新两个字节的数据，值为0x00 才能被更新
	* Called By:    // 
	* Input:        // Mode  If zero, update config byte 84.    If one, update config byte 85.
					// newVol 要更新的数据
	* Output:       // 无
	* Return:       // 见状态表
	* Others:       // 
	*/
	unsigned char UpdateExtra(unsigned char modle,unsigned char newVol);



	


	/*
	* Function:     // 产生32字节的随机数
	* Called By:    // 
	* Input:        // 无
	* Output:       // recRandomNum 用于接收随机数
	* Return:       // 1成功，0失败
	* Others:       // 
	*/
	unsigned char GenerateRandomNum(unsigned char *recRandomNum);




	/*
	* Function:     // 不符合此值(配置寄存器Selector)的所有芯片，进入空闲状态
	* Called By:    // 
	* Input:        // 无
	* Output:       // recRandomNum 用于接收随机数
	* Return:       // 1成功，0失败
	* Others:       // 
	*/
	unsigned char Pause(unsigned char Selector);







	/*
	* Function:     //此命令将生成一个32字节的随机数用于随后GenDig，MAC，HMAC命令
	* Called By:    // 
	* Input:        // InRandomNumLen 输入随机数长度，取值为20或32
					// InRandomNum 20或32字节随机数，可用当时日期
	* Output:       // outRandomNum 用于接收随机数
	* Return:       // 1成功，0失败
	* Others:       // 
	Mode
	0: Combine new random number with NumIn, store in TempKey. Automatically update EEPROM seed only if necessary prior to random number generation. Recommended for highest security.
	1: Combine new random number with NumIn, store in TempKey. Generate random number using existing EEPROM seed, do NOT update EEPROM seed.
	2: Invalid
	3: Operate in pass-through mode and write TempKey with NumIn.
	*/
	unsigned char Nonce(unsigned char Mode,unsigned char InRandomNumLen,unsigned char *InRandomNum,unsigned char *outRandomNum);





	/*
	* Function:     // 锁哪个区域
	* Called By:    // 
	* Input:        // zone  见下
	* Output:       // 无
	* Return:       // 见状态表
	* Others:       
	zone：
	Bit 0: Zero for config zone, 1 for data and OTP zones
	Bits 1-6: Must be zero
	Bit 7: If one, the check of the zone CRC is ignored and the zone is locked, regardless of the state 
			of the memory. Atmel does not recommend using this mode.
	*/
	unsigned char Lock(unsigned char zone);




	/*
	* Function:     // 密码验证
	* Called By:    // 
	* Input:        // slotNum 选择第几个密码进行验证，0-15个密码，
					// RandomNum 32字节的随机数  
					//modle
	* Output:       // recBuf 存放返回的结果
	* Return:       // 0 失败，1成功
	* Others:       // 
	*/
	unsigned char Hmac(unsigned char modle,unsigned char slotNum,unsigned char *recBuf);







	/*
	* Function:     // GenDig
	* Called By:    // 
	* Input:        // slotNum 选择第几个密码进行验证，0-15个密码，
					// Zone 
	* Output:       // 无
	* Return:       // 见状态表
	* Others:       // 
	*/
	unsigned char GenDig(unsigned char Zone,unsigned char slotNum);




	/*
	* Function:     // 获取版本号4字节 
	* Called By:    // 
	* Input:        // 无
	* Output:       // recBuf 版本号接收
	* Return:       // 1成功 0失败
	* Others:       // 
	*/
	unsigned char DevRev(unsigned char *recBuf);




	/*
	* Function:     // 获取版本号4字节 
	* Called By:    // 
	* Input:        // 无
	* Output:       // recBuf 版本号接收
	* Return:       // 见状态表
	* Others:       // 
	*/
	unsigned char DeriveKey(unsigned char Random,unsigned char TargetKey);








	/*
	* Function:     // CheckMac 
	* Called By:    // 
	* Input:        // Mode
					// KeyID
					// ClientChal
					// ClientResp
					//
	* Output:       // recBuf 版本号接收
	* Return:       // 见状态表
	* Others:       // OtherData
	*/
	unsigned char CheckMac(unsigned char Mode,unsigned char KeyID,unsigned char *ClientChal,unsigned char *ClientResp,unsigned char *OtherData);




	/*
	* Function: 	// 获取加密码IC的ID号9BYTE
	* Called By:	// 
	* Input:		// 无
	* Output:		// IdBuf
	* Return:		// 见状态表
	* Others:		// OtherData
	*/
	unsigned char GetID(unsigned char *IdBuf);





protected:
	/*
	* Function:     // I2C开始发送数据
	* Called By:    // 
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 无
	*/
	void I2cStart(void);



	/*
	* Function:     // I2C停止发送数据
	* Called By:    // 
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 无
	*/
	void I2cStop(void);



	/*
	* Function:     // MCU向ATSHA204发送应答信号
	* Called By:    //  
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 接收到一字节的时候发
	*/
	void SendAck(void);



	/*
	* Function:     // MCU向ATSHA204发送无应答信号
	* Called By:    //  
	* Input:        // 无
	* Output:       // 无
	* Return:       // 无
	* Others:       // 接收到最后一字节的时候发
	*/
	void SendNotAck(void);



	/*
	* Function:     // MCU等待ATSHA204芯片应答
	* Called By:    //  
	* Input:        // 无
	* Output:       // 无
	* Return:       // 0代表无应答，1代表有应答
	* Others:       // 无
	*/
	unsigned char WaitAck(void);




	/*
	* Function:     // MCU向ATSHA204发送字节
	* Called By:    //  
	* Input:        // ch  要发送的字节
	* Output:       // 无
	* Return:       // 无
	* Others:       // 高位先发
	*/ 
	void I2cSendByte(unsigned char ch);


	/*
	* Function:     // MCU接收ATSHA204的字节
	* Called By:    //  
	* Input:        // 无
	* Output:       // 无
	* Return:       // 返回接收到的字节
	* Others:       // 无
	*/ 
	unsigned char I2cReceiveByte(void);



	/*
	* Function:     // MCU向ATSHA204写数据包，buf中包含完整的包协议
	* Called By:    // 
	* Input:        // buf 要写入的数据包，length要写入的数据包长度
	* Output:       // 无
	* Return:       // 0 失败，1成功
	* Others:       // 做20次写尝试
	*/
	unsigned char WriteDataToATSHA(unsigned char *buf,unsigned char length);




	/*
	* Function:     // MCU从ATSHA204读数据
	* Called By:    // 
	* Input:        // CmdBuf 要写入的数据(读命令包)，CmdBufLength要写入的数据(读命令)长度，RecBufLength要读出的数据长度
	* Output:       // RecBuf存放读出的数据
	* Return:       // 0 失败，1成功
	* Others:       // 做20次读尝试
	*/
	unsigned char ReadDataFromATSHA(unsigned char *CmdBuf,unsigned char CmdBufLength,unsigned char *RecBuf,unsigned char RecBufLength);





	/*
	* Function:     // CRC16计算函数
	* Called By:    // 
	* Input:        // length data的长度  data 存放要计算的数据
	* Output:       // crc 返回的CRC16
	* Return:       // 无
	* Others:       // 无
	*/
	void sha204c_calculate_crc(unsigned char length, unsigned char *data, unsigned char *crc); 



};
#endif 



