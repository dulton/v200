

#ifndef _JPEG_SNAP_H_
#define _JPEG_SNAP_H_
#include "Video_comm.h"

#include "common.h"


typedef enum _SNAP_CH_TYPE_
{
	SNAP_HD1 = 0,
	SNAP_CIF,
	SNAP_MAX
}SNAP_CH_TYPE;

typedef struct {
	unsigned short			resolution;
	unsigned short			norm;
	int					SnapGroup;
	int					SnapChn;
	int					ViDev;
	int					ViChn;
}JpegSnapPara;

class JpegSnap_S
{
private:

	static JpegSnap_S *m_pInstanse;	
	pthread_mutex_t m_SnapProcessLock;
	VENC_GRP m_VencGrp ;
    VENC_CHN m_VencChn ;
public:
	JpegSnap_S();
	~JpegSnap_S();

	static JpegSnap_S *Instanse();

	
	/*****************************************************************************
	函数功能:创建抓拍通道
	输入参数:enNorm--制式
				  enPicSize--抓拍图片大小
	
	输出参数:无
	返	回	 值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/

	HI_S32 CreateSnapChn(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize);

	/*****************************************************************************
	函数功能:抓拍处理
	输入参数:Para---保存图片的文件句柄
				
	
	输出参数:无
	返	回	 值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	HI_S32 DestroySnapCh();


	HI_S32 JpegSnapProcess(void *Para);
 //  HI_S32 JpegSnapProcess();


private:
	

	/*****************************************************************************
	函数功能:存储jpeg 图片
	输入参数:fpJpegFile--文件指针
				  pstStream--jpeg 流数据
	输出参数:无
	返  回   值:成功返回0，否则返回-1
	使用说明:
	******************************************************************************/
	HI_S32 SaveJpegStream(FILE* fpJpegFile, VENC_STREAM_S *pstStream);


   HI_S32 SAMPLE_COMM_VENC_SaveSnap(VENC_STREAM_S *pstStream, HI_BOOL bSaveJpg, HI_BOOL bSaveThm);
 HI_S32 SAMPLE_COMM_VENC_SaveJPEG(FILE *fpJpegFile, VENC_STREAM_S *pstStream);
};

#endif

