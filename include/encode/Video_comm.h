
#ifndef __VIDEO_COMM_H__
#define __VIDEO_COMM_H__
#include "hi_common.h"
#include "mpi_vi.h"
#include "hi_comm_isp.h"
#include "hi_comm_vb.h"
#include "hi_comm_venc.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"

#include "mpi_isp.h"
#include "mpi_region.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vda.h"
#include "mpi_venc.h"

#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_vpss.h"

#include "hi_sns_ctrl.h"
enum{
	libsns_ov9712_720p = 0,
	libsns_jxh22_720p  = 1,
	libsns_hm1375_720p = 2,
	libsns_ar0130_720p = 3,
};

typedef enum video_rc_e
{
   RC_CBR = 0,
   RC_VBR,
   RC_FIXQP
}VIDEO_RC_E;
int detect_sensor();
int Comm_VI_StartDev(int sensorid);

int  Comm_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize);

int  VideoVencStart(VENC_GRP VencGrp,VENC_CHN VencChn,VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, VIDEO_RC_E enRcMode,unsigned int	framerate,unsigned int bitrate,unsigned int Gop);

int  VideoVencStop(VENC_GRP VencGrp,VENC_CHN VencChn);

int  VideoVencBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);

int  VideoVencUnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);

int  VideoVpssStartGroup(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstVpssGrpAttr);

int  VideoVpssEnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstVpssChnAttr,VPSS_CHN_MODE_S *pstVpssChnMode,VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr);

int  VideoVIBindVpss();

int  VideoVIBindVenc(VENC_GRP VencGrp);

int VideoVIBindUnVenc(VENC_GRP VencGrp);

#endif 

