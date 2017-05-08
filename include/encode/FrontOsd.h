
#ifndef _FRONT_OSD_H_
#define _FRONT_OSD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include "BufferManage.h"
#include "parametermanage.h"
#include "hi_comm_video.h"


#include "mpi_region.h"

#define	MAX_OVERLAY_NUM			8
#define MAX_OSD_HANDLE			12
#define COLOR_VALUE(a, r, g, b) (((a > 0)?0x8000:0) | (((r >> 3) & 0x1f) << 10) | (((g >> 3) & 0x1f) << 5) | ((b >> 3) & 0x1f))


#if 1//ndef RELEASE_APP

#define   HZK_FONT_PATH					"/app/fontfile/hzk16"
#define   ASC_FONT_PATH					"/app/fontfile/asc16"

#else 

#define   HZK_FONT_PATH					"/app/fontfile/hzk16"
#define ASC_FONT_PATH					"/app/fontfile/asc16"

#endif 

#define FONT_W					8
#define FONT_H					16


#define HAND_TIME_CH0			0 
#define HAND_TIME_CH1			1

#define HAND_NAME_CH0			2 
#define HAND_NAME_CH1			3


#define BIND_VNEC_CH0			0 
#define BIND_VNEC_CH1			1 



#define HAND_COVER_BASE			4 







#define CHNAME_720POSD			2 



#define CHNAME_VGAOSD			3 
#define MD_720POSD				4 
#define MD_VGAPOSD				5 
#define SNAP_720POSD			6 
#define TIME_QVGAPOSD			7 
#define CHNAME_QVGAOSD			8 



#define GPROUP_720P				0 
#define GPROUP_VGA				1 
#define GPROUP_QVGA				2 
#define GPROUP_SNAP				3





#define OSD_CHN 				1 
#define OSD_COVER				2 

#define OSD_D1_LEFT_X			0				
#define OSD_D1_LEFT_Y			0

#define OSD_D1_RIGHT_X			330									

#define OSD_HD1_LEFT_X			OSD_D1_LEFT_X				
#define OSD_HD1_RIGHT_X		OSD_D1_RIGHT_X		

#define OSD_QVGA_RIGHT_X		165	

#define OSD_720P_RIGHT_X		960	

#define OSD_1080P_RIGHT_X		640	

#define OSD_CIF_LEFT_X			0				
#define OSD_CIF_RIGHT_X		260	
#define OSD_CIF_LEFT_Y			0


typedef struct TAG_VIDEO_OSD{
	unsigned int x;		/*显示区域x偏移位*/
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned short color;	/*显示区域背景色*/
	int		 BgAlpha;	/*背景透明度,  暂时默认20 ,*/
	int 	resolution;
	unsigned int u32Layer;
	char chnname[17];	/*显示时间时无效*/
}VideoOsd_S;

typedef struct TAG_OSDPARA
{
	int 	handle;
	BITMAP_S stBitmap;
	VideoOsd_S videoosd;
}OsdPara_S;


class FrontOsd
{    

private:
	//编码线程pid
	pthread_t m_OsdThreadPid;

	//线程运行状态标志
	char 	m_OsdThreadRunFlg;

	//编码线程pid
	pthread_t m_OsdThreadPidJpeg;

	//线程运行状态标志
	char 	m_OsdThreadRunJpegFlg;

	//编码前端osd 参数信息
	OsdPara_S	m_osdpara[MAX_OSD_HANDLE];

	OsdPara_S	m_CoverLayerPara[MAX_REC_CHANNEL][MAX_OVERLAY_NUM];

	FILE *hzkfd;
	FILE *ascfd;

	char *hzkptr;
	char *ascptr;

	//日期时间回调函数
	DateTimeCallBack		m_GetSysTime;

	char 				m_second_bak;

	char 				m_second_jpeg_bak;


	CAMERA_PARA		m_CameraSet;
	
	
public:
	unsigned char   m_TimeMode;// 时间的格式:1 表示24 hours, 0 表示am/pm

public:
	FrontOsd(DateTimeCallBack GetDateTime);
	~FrontOsd();   
	void VideoOsdExit(void);	
	int OsdRegionShowCtrl(unsigned int handle,int group,bool show);
	int BindHandToGroup(unsigned int handle,int SrcGroup,int DesGroup);
	int UnBindHandToGroup(unsigned int handle,int SrcGroup,int DesGroup);
	void OsdProcess();

	int VideoOsdInit(void);
	
	int OsdCreateRegion(unsigned int handle,int group,VideoOsd_S *pOsd);
	int OsdDestroyRegion(unsigned int handle,int group);
	int CreateCoverLayerRegion(RECT_S rect,int area);
	int DeleteCoverLayerRegion(int area);


	int ShowVideoOsd_Channel(VideoOsd_S *pPara);
	int ShowVideoOsd_Time(VideoOsd_S *pPara);

	int Osd_show_fixed_text(int hand, char * text, int encode_flag,unsigned int color);
	int osd_find_asc_from_lib(char * tx, unsigned char *buffer);
	int osd_find_hzk_from_lib(char * tx, unsigned char *buffer);
	void osd_to_RGB(unsigned int zcolor, unsigned int bcolor, int len, unsigned char * s, unsigned char *d, int encode_flag,unsigned char *text_flag);
	int UpdateOsdTime(unsigned char	mode,SystemDateTime *psystime);
	int UpdateOsdMD();
	int ChangeOsdTimeByMode(unsigned char timemode);
	int StartOsdProcess();
	int StopOsdProcess();
	int CheckTimeByMode(unsigned char timemode,SystemDateTime *ptime);



};

#endif

