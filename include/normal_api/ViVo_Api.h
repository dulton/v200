/******************************************************************************
  File Name     : ViVo_Api.h
  Version       : Initial Draft 1.0
  Last Modified :
  Description   : Function declare of the vi and vo
  Function List :
  History       :
    Modification: Created file

******************************************************************************/

#ifndef _VIVO_API_H_
#define _VIVO_API_H_

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "hi_comm_region.h"
#include "hi_comm_venc.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
//#include "mpi_vpp.h"
#include "mpi_venc.h"

#include "CommonDefine.h"

#include "common.h"



//#define ADTYPE_VAD5150	 1

//#define KEY_INFO(info) do{printf info;}while(0)

enum OUTPUTMODE_E{
	OUT_ONE = 1,
	OUT_FOUR = 4,
	OUT_SECFOUR = 5,
	OUT_NINE = 9,
	DEC_OUT_ONE = 20,
	DEC_OUT_FOUR = 21,
	DEC_OUT_NINE = 22,
};

enum VENC_MODE_E{
	VENC_1D1 = 0,
	VENC_2D1,
	VENC_4D1,
	VENC_2HD1,
	VENC_4HD1,
	VENC_2D12CIF,
	VENC_4CIF,
	VENC_1080P,
	VENC_720P,
	VENC_VGA,
	VENC_QVGA,
	VENC_MAX,
};





#endif
