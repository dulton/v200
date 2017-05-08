
#ifndef __VIDEO_ISP_H__
#define __VIDEO_ISP_H__

#include "common.h"
#include "hi_type.h"
#include "mpi_isp.h"
#include "hi_comm_isp.h"
#include "Video_comm.h"

class CVideoISP{
	public:
		
		CVideoISP();
		~CVideoISP();		
		/*****************************************************************************
		函数名称:ISP_Run
		函数功能:
		输入参数:无
		输出参数:无
		返	回	 值: 0: 成功-1: 失败
		使用说明:外部调用
		
		******************************************************************************/

		int ISP_Run();
		int StopIsp();
		int SetVideoSaturation(unsigned int value);
		#if 0
		int SetVideoBrightness(unsigned int value);
		#endif
		int SetVideoChnAnaLog(int brightness,int contrast,int saturation);
		int H42_ISPseting();
		int SetAntiFlickerAttr(unsigned char enable,unsigned short Frequency);
	private:

		pthread_t m_IspPid;

};



#endif 

