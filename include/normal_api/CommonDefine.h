
/******************************************************************************
  File Name     : CommonDefine.h
  Version       : Initial Draft
  Last Modified :
  Description   : the functions define
  Function List :
  History       :

******************************************************************************/
#ifndef _COMMON_DEFINE_H
#define _COMMON_DEFINE_H

#define SYSTEM_PAL			0
#define SYSTEM_NTSC			1

#define MAX_REC_MODE				4



//分辨率
typedef enum{
	RES_D1 = 0,
	RES_HD1,
	RES_CIF,
	RES_QCIF,
	RES_1080P,
	RES_720P,
	RES_VGA,
	RES_QVGA,
}Resolution_E;

typedef struct DATA_GPS 
{ 
	char				cGpsStatus; 				/*gps是否有效标识*/ 
	char				cSpeedUnit; 				/*速度单位*/ 
	unsigned short		usSpeed;				/*速度值*/ 
	char				cLatitudeDegree;		/*纬度值的度*/ 
	char				cLatitudeCent; 			/*纬度值的分*/ 
	char				cLongitudeDegree;		/*经度值的度*/ 
	char				cLongitudeCent;			/*经度值的分*/ 
	long				lLatitudeSec;			/*纬度值的秒*/ 
	long				lLongitudeSec;			/*经度值的秒*/ 
	unsigned short		usGpsAngle;				/*gps夹角*/ 
	char				cDirectionLatitude;		/*纬度的方向*/ 
	char				cDirectionLongitude;		// 经度的方向 
	char				reserved[4]; 
}struGPSData;


typedef struct _stru_G_Sensor_
{
	short					x;
	short					y;
	short					z;
	short 					unit;			//> 0 加速度有效，实际的加速度计算方法是加速度读数除以此值。 
											//= 0 表示本加速度无效，即没有安装加速度传感器，或者接收的数据无效 
											//< 0 本结构无效，非法值

}struGSensor;

typedef struct DATA_GYRO_SENSOR
{
	short		AccelerateValue;				//转角加速度值
	short		Unit;						//单位
}struGyroSensor;


typedef struct MCU_DATA_STATUS
{
	unsigned char		AccStatus;			//车钥匙信号，1-钥匙信号有效，0-钥匙信号无效
	unsigned char		Brake;				//刹车信号，1-有刹车信号，0-没有刹车信号
 	unsigned char		Winker;				//转向信号，1-左转，2-右转，3-两个都有效，0-无效
	unsigned char		TempValid;			//温度值是否有效，1-有效，0-无效
	unsigned char		TempUnit;			//温度单位，0-摄氏度，1-华氏度
	unsigned char		SpeedValid;			//速度值是否有效，1-有效，0-无效
	unsigned char  	SpeedUnit;			//速度单位；0-千米/小时，1-英里/小时
	unsigned char		reserve;			//
	short			Temperature;		//
	short			speed;				//
}struStatus;


typedef struct BINARY_HEAD
{
	unsigned short			DataType;		//数据类型
	unsigned short			DataLen;		//二进制数据的长度
	unsigned long long			Pts;				//时间戳信息，单位为毫秒
}struBinHead;

typedef struct DATA_COMMON
{
	//DateType = 0x0f
	struBinHead 		BinaryHead;
	struGSensor		GSensor;
	struGyroSensor	GyroSensor;
	struStatus	 		Status;
	struGPSData		Gps; 
}struMcuData;

typedef struct DATA_IN_TIME
{
	//DateType = 0x03
	struBinHead 		BinaryHead;
	struGSensor 		GSensor;
	struGyroSensor 	GyroSensor;
}struDataRealtime;

//extern "C" unsigned long Mscount(unsigned long Count);


// 用于获取H.264 数据NAL单元信息
#define MAX_PKT_NUM_IN_STREAM 10
typedef enum nalu_type { 
	NALU_PSLICE = 1, 
	NALU_ISLICE = 5, 
	NALU_SEI = 6, 
	NALU_SPS = 7, 
	NALU_PPS = 8, 
} nalu_type_t;

typedef struct venc_pkt {
	unsigned len;
	nalu_type_t type;
} venc_pkt_t;

typedef struct venc_stream {
	venc_pkt_t pkt[MAX_PKT_NUM_IN_STREAM];
	unsigned pkt_cnt; 
	unsigned seq;
} venc_stream_t;




#endif
	
