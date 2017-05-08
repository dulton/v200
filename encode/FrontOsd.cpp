
#include "His_Api_define.h"
#include "FrontOsd.h"
#include "CommonFunction.h"
#include "ModuleFuncInterface.h"
#include "common.h"


extern int startupdate;
extern int GetSysTime(SystemDateTime *pSysTime);
void * OsdShowEntry(void * para)
{
	FrontOsd *pFrontOsd = (FrontOsd *)para;
	printf("function: %s threadid %d  ,line:%d\n",__FUNCTION__, (unsigned)pthread_self(),__LINE__);
	if(pFrontOsd != NULL)
	{
		pFrontOsd->OsdProcess();
	}	
	return NULL;
}

int CreatOverLay(unsigned int handle,int BindVecnChn,int x,int y,int width,int height)
{
	HI_S32 i=handle;
	HI_S32 s32Ret;
	MPP_CHN_S stChn;
	RGN_ATTR_S stRgnAttr;
	RGN_CHN_ATTR_S stChnAttr;
	
	/* Add cover to vpss group */
	stChn.enModId  = HI_ID_VENC;
	stChn.s32DevId = 0;
	stChn.s32ChnId = BindVecnChn;	

	stRgnAttr.enType = OVERLAY_RGN;
	stRgnAttr.unAttr.stOverlay.enPixelFmt		= PIXEL_FORMAT_RGB_1555;
	stRgnAttr.unAttr.stOverlay.stSize.u32Width	= width;
	stRgnAttr.unAttr.stOverlay.stSize.u32Height = height;
	stRgnAttr.unAttr.stOverlay.u32BgColor		= 0;

	s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}

	stChnAttr.bShow  = HI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = x;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = y;
	stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 0;
	stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 128;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer	  = 0;
	
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable  = HI_FALSE;

	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod 	= MORETHAN_LUM_THRESH;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn	= HI_FALSE;


	//stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 128;
	//stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 320;
	

	s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
	if(s32Ret != HI_SUCCESS)
	{
		printf("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}
	printf("[x:%d,y:%d,w:%d,y:%d]succcess! s32Ret: 0x%x.\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X,\
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y,\
		stRgnAttr.unAttr.stOverlay.stSize.u32Width,\
		stRgnAttr.unAttr.stOverlay.stSize.u32Height, \
		s32Ret);

	return HI_SUCCESS;
	
}

int CreateTimeRegion(int TimeMode,unsigned int handle,int BindVecnChn)
{
	int group = 0;
	int x,y,width,height;
	if(handle == 0)
	{
		if(TimeMode == 0)
		{
			x = 960;
			width= FONT_W*19*2;//2012-01-07 23:00:00
		}
		else
		{
			x = 920;			
			width= FONT_W*23*2;//2012-01-07 12:00:00 am
		}		
		y = 0;		
		height= FONT_H*2;
	
		
		group = GPROUP_720P;
	}
	else if(handle == 1)
	{
		if(TimeMode == 0)
		{
			//x = 470;
			x = 330;
			width= FONT_W*19*2;//2012-01-07 12:00:00
		}
		else
		{
			//x = 460;
			x = 280;				
			width= FONT_W*23*2;//2012-01-07 12:00:00 am
		}			
		y = 0;		
		height= FONT_H*2;
	}
	else if(handle == 2)
	{
		if(TimeMode == 0)
		{
			x = 140;
			width= FONT_W*23;//2012-01-07 12:00:00 am
		}
		else
		{
			x = 158;
			width= FONT_W*19;//2012-01-07 12:00:00
		}			
		y = 0;		
		height= FONT_H;		
	}
	CreatOverLay(handle,BindVecnChn,x,y,width,height);
	return 0;
}


/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
void FrontOsd::VideoOsdExit(void)
{
	if(hzkfd != NULL)
	{
		fclose(hzkfd);
		hzkfd = NULL;
	}
	
	if(ascfd != NULL)
	{
		fclose(ascfd);
		ascfd = NULL;
	}

	if(ascptr != NULL)
	{
		free(ascptr);
		ascptr = NULL;
	}

	if(hzkptr != NULL)
	{
		free(hzkptr);
		hzkptr = NULL;
	}
	

}

/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
FrontOsd::FrontOsd(DateTimeCallBack GetDateTime)
{
	
	m_GetSysTime = GetDateTime;
	memset(m_osdpara,0x0,sizeof(m_osdpara));
}

/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
FrontOsd::~FrontOsd()
{
}

/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
int FrontOsd::VideoOsdInit(void)
{
	int	retval = 0;
	int lSize = 0;
	hzkfd = NULL;
	ascfd = NULL;
	hzkptr = NULL;
	ascptr = NULL;
	if((hzkfd = fopen(HZK_FONT_PATH, "rb")) == NULL)
	{
		DEBUG_INFO("Front osd Open font lib hzk16 failed !\n");
		return -1;
	}

	//*获取文件大小.   
	fseek(hzkfd, 0, SEEK_END);   
	lSize = ftell(hzkfd);   
	printf("File size = %d\n", lSize);   
	rewind(hzkfd);   
	
	//*分配内存以包含整个文件 
	hzkptr = (char*)malloc(lSize);
	if(hzkptr == NULL)   
	{   
		perror("Couldn't allocate memory\n");   
		exit(2);   
	}    
	retval = fread(hzkptr,1,lSize,hzkfd);   
	//printf("Read size = %d\n", retval); 
	
	if((ascfd = fopen(ASC_FONT_PATH, "rb")) == NULL)
	{
		printf("Front osd Open font lib asc16 failed !\n");
		return -1;
	}

	fseek(ascfd, 0, SEEK_END);   
	lSize = ftell(ascfd);   
	//printf("File size = %d\n", lSize);   
	rewind(ascfd);   

	//*分配内存以包含整个文件 
	ascptr = (char*)malloc(lSize);
	if(ascptr == NULL)   
	{   
		if(hzkptr != NULL) 
		{
			free(hzkptr);
			hzkptr = NULL;
		}
		perror("Couldn't allocate memory\n");   
		exit(2);   
	}    
	retval = fread(ascptr,1,lSize,ascfd);   
	//printf("Read size = %d\n", retval); 
	
	
	return 0;
}



int FrontOsd::OsdCreateRegion(unsigned int handle,int group,VideoOsd_S *pOsd)
{
	if(pOsd == NULL)
	{
		return -1;
	}
	HI_S32 s32Ret = HI_FAILURE;
	RGN_HANDLE RgnHandle;
	RGN_ATTR_S stRgnAttr;
    MPP_CHN_S stChn ;   
	RGN_CHN_ATTR_S stChnAttr;
	
	RgnHandle = handle;
	
	stRgnAttr.enType = OVERLAY_RGN;
	stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRgnAttr.unAttr.stOverlay.stSize.u32Width	= (pOsd->width+ 15) / 16 * 16; // 2X对齐
	stRgnAttr.unAttr.stOverlay.stSize.u32Height = (pOsd->height+ 15) / 16 * 16; // 2X对齐
	stRgnAttr.unAttr.stOverlay.u32BgColor = pOsd->color;



	
	s32Ret = HI_MPI_RGN_Create(RgnHandle, &stRgnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_Create (%d) failed with %#x!\n", \
			   RgnHandle, s32Ret);
		return HI_FAILURE;
	}	

	stChn.enModId = HI_ID_VENC;
	stChn.s32DevId = 0;
	stChn.s32ChnId = group;
	
	memset(&stChnAttr,0,sizeof(stChnAttr));
	stChnAttr.bShow = HI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X =(pOsd->x + 15) / 16 * 16; // 4X对齐
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y =(pOsd->y + 15) / 16 * 16;
	stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0;//(pOsd->BgAlpha == -1)? 0:50;
	stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = handle;
	
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_TRUE;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod  =MORETHAN_LUM_THRESH ;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 150;
	
	s32Ret = HI_MPI_RGN_AttachToChn(RgnHandle, &stChn, &stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_AttachToChn [%d,%d] failed with %#x!\n", handle, group, s32Ret);
		return HI_FAILURE;
	}	
	printf("HI_MPI_RGN_AttachToChn [%d,%d] success with %#x!\n", handle, group, s32Ret);
	return HI_SUCCESS;
}

int FrontOsd::OsdDestroyRegion(unsigned int handle,int group)
{
	HI_S32 s32Ret = HI_FAILURE;
	MPP_CHN_S stChn ;
	RGN_HANDLE RgnHandle = handle; 
	
	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = group;
	stChn.s32ChnId = 0;
	


	s32Ret = HI_MPI_RGN_Destroy(RgnHandle);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_Destroy [%d] failed with %#x\n",\
				RgnHandle, s32Ret);
	}
	return HI_SUCCESS;

}

int FrontOsd::OsdRegionShowCtrl(unsigned int handle,int group,bool show)
{
	MPP_CHN_S stChn;
	RGN_CHN_ATTR_S stChnAttr;
	HI_S32 s32Ret;
	RGN_HANDLE RgnHandle = handle;
	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = group;
	stChn.s32ChnId = 0;
	
	s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",\
			   RgnHandle, s32Ret);
		return HI_FAILURE;
	}
	//printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X==:%d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
	//printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y==:%d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);

	if(show == true)
	{
		stChnAttr.bShow = HI_TRUE;
	}
	else
	{
		stChnAttr.bShow = HI_FALSE;
	}
	s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHandle,&stChn,&stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n",\
			   RgnHandle, s32Ret);
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}
int FrontOsd::ShowVideoOsd_Channel(VideoOsd_S *pPara)
{
	if(pPara == NULL)
	{
		return -1;
	}
	char title[32]={0x00};
	if(strlen(pPara->chnname)>16)
	{
		sprintf(title,"%s","title error");;
	}
	
	sprintf(title,"%s",pPara->chnname);
	

	int textlen = strlen(title);
	CreatOverLay(HAND_NAME_CH0,BIND_VNEC_CH0,16,0,FONT_W*textlen*2,FONT_H*2);
	Osd_show_fixed_text(HAND_NAME_CH0, title,RES_720P,0);
	
	CreatOverLay(HAND_NAME_CH1,BIND_VNEC_CH1,8,0,FONT_W*textlen,FONT_H);
	Osd_show_fixed_text(HAND_NAME_CH1, title,RES_QVGA,0);	
	
	return 0;

}
int FrontOsd::ShowVideoOsd_Time(VideoOsd_S *pPara)
{

	CreateTimeRegion(m_TimeMode,HAND_TIME_CH0,BIND_VNEC_CH0);
	CreateTimeRegion(m_TimeMode,HAND_TIME_CH1,BIND_VNEC_CH1);	
	//CreateTimeRegion(TIME_QVGAPOSD);
}
int FrontOsd::CreateCoverLayerRegion(RECT_S rect,int area)
{

	
	if(area > 3)
	{
		return -1;
	}
	
    HI_S32 s32Ret = HI_FAILURE;

    RGN_HANDLE coverHandle;
    RGN_ATTR_S stCoverAttr;
    MPP_CHN_S stCoverChn;
    RGN_CHN_ATTR_S stCoverChnAttr;



    coverHandle = HAND_COVER_BASE+area;
    stCoverAttr.enType = COVER_RGN;
    s32Ret = HI_MPI_RGN_Create(coverHandle, &stCoverAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_Create failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
	int x,y,w,h;

	x = rect.s32X*2;
	y = rect.s32Y*3/2;
	w = rect.u32Width * 2;
	h = rect.u32Height*3/2;
	
    stCoverChn.enModId = HI_ID_VPSS;
    stCoverChn.s32ChnId = 0;
    stCoverChn.s32DevId = 0;

    stCoverChnAttr.bShow = HI_TRUE;
    stCoverChnAttr.enType = COVER_RGN;
	stCoverChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
    stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = (x+ 1) / 2 * 2; // 2X对齐
    stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = (y+3) / 4 * 4; // 4X对齐
    stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = (w+ 1) / 2 * 2; // 2X对齐
    stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = (h+3) / 4 * 4; // 4X对齐
	
    stCoverChnAttr.unChnAttr.stCoverChn.u32Color = 0xffffffff;
    stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = 0;
    s32Ret = HI_MPI_RGN_AttachToChn(coverHandle, &stCoverChn, &stCoverChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_AttachToChn failed with %#x!\n", s32Ret);     
    }
	/*
	printf("-------------------------\nCreateCoverLayerRegion\n");
	printf("x:%d,y:%d,w:%d,h:%d  area:%d\n",stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X,\
		stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y,\
		stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width,\
		stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height,area);
	printf("-------------------------\n");
	*/
	
	return 0;
}
int FrontOsd::DeleteCoverLayerRegion(int area)
{
	if(area > 3)
	{
		return -1;
	}
	RGN_HANDLE coverHandle = HAND_COVER_BASE+area;
	HI_MPI_RGN_Destroy(coverHandle);	
	return 0;
}


int FrontOsd::BindHandToGroup(unsigned int handle,int SrcGroup,int DesGroup)
{
	
	HI_S32 s32Ret = HI_FAILURE;
	
    MPP_CHN_S stChn ;   
	RGN_CHN_ATTR_S stChnAttr;



	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = SrcGroup;
	stChn.s32ChnId = 0;
	
	memset(&stChnAttr,0,sizeof(stChnAttr));
	
    s32Ret = HI_MPI_RGN_GetDisplayAttr(handle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",\
               handle, s32Ret);
        return HI_FAILURE;
    }

	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = DesGroup;
	stChn.s32ChnId = 0;

	s32Ret = HI_MPI_RGN_AttachToChn(handle, &stChn, &stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_RGN_AttachToChn (%d) failed with %#x!\n",\
			   handle, s32Ret);
		return HI_FAILURE;
	}	
	return HI_SUCCESS;

}

int FrontOsd::UnBindHandToGroup(unsigned int handle,int SrcGroup,int DesGroup)
{
	
	HI_S32 s32Ret = HI_FAILURE;	
    MPP_CHN_S stChn ;  
	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = DesGroup;
	stChn.s32ChnId = 0;

	return HI_SUCCESS;

}

/*******************************************************************************************************
	Function: 用指定的颜色把中英文文字点阵数据转换成RGB格式的数据   
	Description:   
	Calls:		 
  	Called By:     
	parameter:zcolor:指定字颜色,bcolor:指定字背景色，len:字符串长度,s:字库点阵数组,
			**d:RGP模式的数据指针,text_flag:中英文点阵数据结构的区分标志数组
  	Return: 字符串的RGB格式的数据      
  	author:
*********************************************************************************************************/
void FrontOsd::osd_to_RGB(unsigned int zcolor, unsigned int bcolor, int len, unsigned char * s, unsigned char *d, int encode_flag,unsigned char *text_flag)
{
	int m,n,i,j,k;
	int textlen;
	textlen = len;
	unsigned char R = zcolor&0x1f;
	unsigned char G = zcolor>>5&0x1f;
	unsigned char B = zcolor>>10&0x1f;
	
	unsigned char *flag=text_flag;//记录每行中英文处理标志的位置
	
	if((encode_flag == RES_D1)||(encode_flag == RES_720P))
	{
		for(m = 0;m<16;m++)//行
		{
			for(n= 0;n<2;n++)
			{
				for(i=0; i<textlen; i++)
				{
					if(*flag++ != 0)//处理中文字符
					{
						for(j=0;j<2;j++)
						{
							for(k=0;k<8;k++) //象素
				 			{ 
				 				if(((s[m*2+j+i*16]>>(7-k))&0x1)!=0) //字色
				 				{
									*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
									d += 2;
									*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
									d += 2;
								}
								else
								{
									*d++ = bcolor;//背景色(0x03:绿)
									*d++ = bcolor;
				
									*d++ = bcolor;
									*d++ = bcolor;

								}
				 			} 
						}
						/************************
						一个中文字符占两个字节,因此中英文区分标志位和
						字符长度控制变量都要做相应的增加
						*************************/
						flag++;
						i++;
					}
					else//处理英文字符
					{
						for(j=0;j<8;j++) 
			 			{ 
			 				if(((s[m+i*16]>>(7-j))&0x1)!=0) 
			 				{
								*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
								d += 2;
								*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
								d += 2;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;
								
								*d++ = bcolor;
								*d++ = bcolor;
							}
			 			}
					}
				}
				//处理下一行开始前，恢复中英文标志flag
				flag=text_flag;
			}
		}
	}
	
	else if(encode_flag == RES_HD1)
	{
		for(m = 0;m<16;m++)
		{
			for(i=0; i<textlen; i++)
			{
				if(*flag++ != 0)
				{
						for(j=0;j<2;j++)
						{
							for(k=0;k<8;k++) 
				 			{ 
				 				if(((s[m*2+j+i*16]>>(7-k))&0x1)!=0) 
				 				{
									*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
									d += 2;
									*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
									d += 2;
								}
								else
								{
									*d++ = bcolor;
									*d++ = bcolor;
									
									*d++ = bcolor;
									*d++ = bcolor;

								}
				 			} 
						}
						//一个中文字符占两个字节
						flag++;
						i++;
					}
					else
					{
						//处理英文字符
						for(j=0;j<8;j++) 
			 			{ 
			 				if(((s[m+i*16]>>(7-j))&0x1)!=0) 
			 				{
								*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
								d += 2;
								*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
								d += 2;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;
							
								*d++ = bcolor;
								*d++ = bcolor;

							}
			 			}
					}	
				}
				//处理下一行开始前，恢复中英文标志flag
				flag=text_flag;
		}
	}
	
	else if((encode_flag == RES_VGA)||(encode_flag == RES_QVGA))
	{
		for(m = 0;m<16;m++)
		{
			for(i=0; i<textlen; i++)
			{
				if(*flag++ != 0)
				{
					for(j=0;j<2;j++)
					{
						for(k=0;k<8;k++) 
			 			{ 
			 				if(((s[m*2+i*16+j]>>(7-k))&0x1)!=0) 
			 				{
								*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
								d += 2;
							}
							else
							{
								*d++ = bcolor;
								*d++ = bcolor;

							}
			 			} 
					}
					//一个中文字符占两个字节
					flag++;
					i++;
				}
				else//处理英文字符	
				{
					for(j=0;j<8;j++) 
		 			{ 
		 				if(((s[m+i*16]>>(7-j))&0x1)!=0) 
		 				{
							*(unsigned short *)d = 0x8000 | (R << 10) | (G << 5) | (B);
							d += 2;
						}
						else
						{
							*d++ = bcolor;
							*d++ = bcolor;
						}
		 			}
				}
			}	
			//处理下一行开始前，恢复中英文标志flag
			flag=text_flag;
		}
	}
}

/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
int FrontOsd::osd_find_asc_from_lib(char * tx, unsigned char *buffer)
{
	int chrlen = strlen(tx);
	char * t = ( char *)tx;
	int i;
	
	ASSERT(buffer != NULL);
	ASSERT(tx != NULL);
	if(ascfd == NULL)
	{
		return -1;
	}
	for(i=0; i<chrlen; i++)
	{
		//fseek(ascfd, (*t)*16,0);	
		//fread(&buffer[i*16],16,1,ascfd);
		memcpy(&buffer[i*16],&ascptr[(*t)*16],16);
		t++;
	}

	return 0;
}

/*********************************************************************
函数功能:
输入参数:无
输出参数:无
返  回   值:成功返回0，否则返回-1
使用说明:
********************************************************************/
int FrontOsd::osd_find_hzk_from_lib(char * tx, unsigned char *buffer)
{
	int chrlen = strlen(tx);
	char * t = ( char *)tx;
 	unsigned char qh,wh; 
 	unsigned long location;
	int i;

	ASSERT(buffer != NULL);
	ASSERT(tx != NULL);
	if(hzkfd == NULL)
	{
		return -1;
	}
	chrlen /= 2;
	for(i = 0;i<chrlen;i++)
	{ 
		qh=*t-0xa0; 
		wh=*(t+1)-0xa0; 
		location=(94*(qh-1)+(wh-1))*32; 
	// 	fseek(hzkfd,location,SEEK_SET); 
	// 	fread(&buffer[i*32],32,1,hzkfd);
		memcpy(&buffer[i*32],&hzkptr[location],32);
		t+=2;
	} 

	return 0;
}

/*********************************************************************
函数功能:
输入参数: 无
输出参数: 无
返 回 值: 成功返回0，否则返回-1
使用说明:
********************************************************************/
int FrontOsd::Osd_show_fixed_text(int hand, char * text, int encode_flag,unsigned int color)
{
	int chrlen;
	HI_S32 s32Ret = HI_FAILURE;
	unsigned char  lib[512];
	unsigned char * pdata;
	
	
	ASSERT(text != NULL);
	chrlen = strlen(text);
	
	if((encode_flag == RES_720P))
	{
		m_osdpara[hand].stBitmap.u32Height =FONT_H*2;
		m_osdpara[hand].stBitmap.u32Width = FONT_W*chrlen*2;
	}
	else if(encode_flag == RES_VGA)
	{
		m_osdpara[hand].stBitmap.u32Height =FONT_H;
		m_osdpara[hand].stBitmap.u32Width = FONT_W*chrlen;
	}	
	else if(encode_flag == RES_QVGA)
	{
		m_osdpara[hand].stBitmap.u32Height =FONT_H;
		m_osdpara[hand].stBitmap.u32Width = FONT_W*chrlen;
	}

	m_osdpara[hand].stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
	m_osdpara[hand].stBitmap.pData = (char *)malloc(m_osdpara[hand].stBitmap.u32Width*m_osdpara[hand].stBitmap.u32Height*2);
	pdata = (unsigned char *)m_osdpara[hand].stBitmap.pData;
	
	memset(lib,0,512);//*读取字库文件

	/**********
	此处开始分别处理中英文差异问题，要让中英文混合显示处理，从此处开始。
	************/
	
	char tmptext[3];//*该变量用于存储当前要处理的字符串，英文的长度为1，中文的长度为2。
	unsigned char text_language_flag[30]; //*记录lib中的中英文标志	
	unsigned int text_current=0; //*记录当前中英文标志位置
	unsigned int lib_current=0;	//*记录当前lib位置	
	
	memset(text_language_flag,0,30);
	while(*text)
	{
		if(*text< 0xa0)
		{
			tmptext[0]=*text++;
			tmptext[1]='\0';
			osd_find_asc_from_lib(tmptext, &lib[lib_current]);//*获取英文字库
			lib_current += 16;
			text_language_flag[text_current++]=0;
		}
		else
		{
			tmptext[0]=*text++;
			tmptext[1]=*text++;
			tmptext[2]='\0';
			if(tmptext[1] == 0)
				break;
			osd_find_hzk_from_lib(tmptext, &lib[lib_current]);//*获取中文字库
			lib_current +=32;
			text_language_flag[text_current++]=1;
			text_language_flag[text_current++]=1;
		}
		
		//printf("tmptext is :%s\n",tmptext);//*打印提取中英文字符串信息
	}
	
	//*将中英文点阵转换为RGB数据
	if(color == 0)
	{	
		osd_to_RGB(0x7fff,0x0,chrlen,lib,pdata,encode_flag,text_language_flag); 
	}
	else
	{
		osd_to_RGB(color,0x0,chrlen,lib,pdata,encode_flag,text_language_flag); 
	}
	
	//*为指定的OSD区域设置显示内容，其中显示内容在Bitmap中。
	s32Ret = HI_MPI_RGN_SetBitMap(hand,&m_osdpara[hand].stBitmap);
    if (NULL != pdata)
    {
        free(m_osdpara[hand].stBitmap.pData);
        pdata = NULL;
		m_osdpara[hand].stBitmap.pData  =NULL;
    }
	
	
	return 0;
}
void FrontOsd::OsdProcess()
{

	printf(" OsdProcess pid:%d !!!!\n",(unsigned)pthread_self());
	pthread_detach(pthread_self());/*退出线程释放资源*/
	char second_bak =60;
	COMMON_PARA  CommPara;

	SystemDateTime datetime;
	
	while(m_OsdThreadRunFlg)
	{

		if(startupdate == 0)
		{
			printf("system update thread exit file:%s line:%d pid:%d  \n",__FILE__,__LINE__,getpid());
			VideoOsdExit();
			pthread_exit(0);
		}

		m_GetSysTime(&datetime);
		//printf("m_GetSysTime:%d:%d:%d\n",datetime.hour,datetime.minute,datetime.second);
		if(second_bak != datetime.second)
		{
			second_bak = datetime.second;
			PubGetSysParameter(SYSCOMMON_SET, &CommPara);
			ChangeOsdTimeByMode(CommPara.m_uTimeMode);
			UpdateOsdTime(CommPara.m_uDateMode,&datetime);
			
		}
		
		/*
		去掉 MD	叠加信息
		UpdateOsdMD();
		*/
		usleep(200000);
	}
	
}
int FrontOsd::UpdateOsdTime(unsigned char	mode,SystemDateTime *psystime)
{
	char realtime[64] = {0x0};
	int ret = CheckTimeByMode(m_TimeMode,psystime);
	switch(mode)
	{
		case 0://dd-mm-yyyy
			sprintf(realtime, "%02d-%02d-20%02d %02d:%02d:%02d", psystime->mday, 
					psystime->month, psystime->year, psystime->hour, psystime->minute,psystime->second);
			//sprintf(realtime, "%02d-%02d-20%02d %02d:%02d:%02d", psystime->mday, 
					//psystime->month, psystime->year, psystime->hour, psystime->minute,psystime->second);
			break;
			
		case 2://mm-dd-yyyy
			sprintf(realtime, "%02d-%02d-20%02d %02d:%02d:%02d", psystime->month, 
					psystime->mday, psystime->year, psystime->hour, psystime->minute, psystime->second);
			//sprintf(realtime, "%02d-%02d-20%02d %02d:%02d:%02d", psystime->month, 
					//psystime->mday, psystime->year, psystime->hour, psystime->minute, psystime->second);
			break;				
		case 1://yyyy-mm-dd
		default:	
			sprintf(realtime, "20%02d-%02d-%02d %02d:%02d:%02d",psystime->year, 
					psystime->month, psystime->mday, psystime->hour, psystime->minute,psystime->second);
			//sprintf(realtime, "20%02d-%02d-%02d %02d:%02d:%02d",psystime->year, 
					//psystime->month, psystime->mday, psystime->hour, psystime->minute,psystime->second);
			break;	
	}
	if(ret == 0)
	{
		strcat(realtime," AM");		
	}
	else if(ret == 1)
	{
		strcat(realtime," PM");		
	}		
	Osd_show_fixed_text(BIND_VNEC_CH0, realtime,RES_720P,0);
	Osd_show_fixed_text(HAND_TIME_CH1, realtime,RES_720P,0); 
/*
	MPP_CHN_S stMppChn =  {0};
	RGN_CHN_ATTR_S stOsdChnAttr = {0};
    stMppChn.enModId  = HI_ID_VENC;
    stMppChn.s32DevId = 0;
    stMppChn.s32ChnId = BIND_VNEC_CH0;
    HI_MPI_RGN_GetDisplayAttr(HAND_TIME_CH0, &stMppChn, &stOsdChnAttr);

	
	printf("[x:%d,y:%d,w:%d,y:%d]succcess! s32Ret: 0x%x.\n",stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32X,\
		stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y,\
		stOsdChnAttr.unAttr.stOverlay.stSize.u32Width,\
		stOsdChnAttr.unAttr.stOverlay.stSize.u32Height, \
		s32Ret);

    stMppChn.enModId  = HI_ID_VENC;
    stMppChn.s32DevId = 0;
    stMppChn.s32ChnId = BIND_VNEC_CH1;
    HI_MPI_RGN_GetDisplayAttr(HAND_TIME_CH1, &stMppChn, &stOsdChnAttr);

	int i=0;
	RECT_S astVencOsdLumaRect[3];

	VPSS_REGION_INFO_S stLumaRgnInfo;
	HI_U32 au32LumaData[16];
	stLumaRgnInfo.u32RegionNum =2;
	stLumaRgnInfo.pstRegion = astVencOsdLumaRect;
	HI_MPI_VPSS_GetRegionLuma(0, 0, &(stLumaRgnInfo), au32LumaData, -1);
	for (i = 0; i < stLumaRgnInfo.u32RegionNum; ++i)
   {
	   
	   printf("================[%d,%d,%d,%d][%d]\n",stLumaRgnInfo.pstRegion[i].s32X,\
	   	stLumaRgnInfo.pstRegion[i].s32Y,\
	   	stLumaRgnInfo.pstRegion[i].u32Width,\
	   	stLumaRgnInfo.pstRegion[i].u32Height,\
	   	au32LumaData[i]);
	   
   }
	*/

	

	
	return 0;
}
//extern int Speed;
int FrontOsd::UpdateOsdMD()
{
	#if 0
	char content[16]={0};
	sprintf(content,"%dKB/S",Speed);
	//Osd_show_fixed_text(MD_720POSD, content,RES_720P,COLOR_VALUE(1,0,250,0));
	Osd_show_fixed_text(MD_720POSD, content,RES_720P,0);
	#endif
	//printf("===========content:%s\n",content);
	//OsdRegionShowCtrl(MD_720POSD,GPROUP_720P,true);
	#if 1
	static time_t time_bak = time(NULL);
	static bool md_show =  false;
	time_t current = time(NULL);
	int MdStatus = GetMotionInfo(0);


	if((md_show == false)&&(MdStatus == 1))
	{
		md_show = true;
		time_bak = current;
		//printf("show -------------------------------------------\n");
		OsdRegionShowCtrl(MD_720POSD,GPROUP_720P,true);
		OsdRegionShowCtrl(MD_VGAPOSD,GPROUP_VGA,true);
		return 0;
		//show;
	}	
	if((md_show == true)&&(MdStatus == 0)&&((current-time_bak)>3))
	{
		md_show = false;
		time_bak = current;
		OsdRegionShowCtrl(MD_720POSD,GPROUP_720P,false);
		OsdRegionShowCtrl(MD_VGAPOSD,GPROUP_VGA,false);

		//printf("hide -------------------------------------------\n");		
		//hide;
	}	
	#endif
	
	
	return 0;
}
int FrontOsd::ChangeOsdTimeByMode(unsigned char	timemode)
{
	if(m_TimeMode == timemode)
	{
		return 0;
	}
	
	m_TimeMode =timemode;	
	
	OsdDestroyRegion(HAND_TIME_CH0,GPROUP_720P);	
	OsdDestroyRegion(HAND_TIME_CH1,GPROUP_VGA);

	
	ShowVideoOsd_Time(NULL);

	return 0;
}

int FrontOsd::StartOsdProcess()
{
	m_OsdThreadRunFlg = 1;
	if(pthread_create(&m_OsdThreadPid, NULL, OsdShowEntry, (void *)(this)) < 0)//创建线程
	{
		printf("OsdShowEntry failed \n");
	}
	return 0;
}

int FrontOsd::StopOsdProcess()
{
	m_OsdThreadRunFlg = 0;

	if(m_OsdThreadPid > 0)
	{
		pthread_join(m_OsdThreadPid, NULL);
		m_OsdThreadPid = 0;
	}
	HI_MPI_RGN_Destroy(HAND_TIME_CH0);
	HI_MPI_RGN_Destroy(HAND_TIME_CH1);
	HI_MPI_RGN_Destroy(HAND_NAME_CH0);
	HI_MPI_RGN_Destroy(HAND_NAME_CH1);


	printf("front osd thread exit sucessful!\n");
	return 0;
}
/****************************************************************************
**函数名称	: CheckTimeByMode 
**函数功能	: 检测24 和12小时制时间显示方式，24小时制时间不变。
				12小时制:hour大于12(13 ,14 ,.......)减去12。
				
**输入参数	:timemode :模式0: 24小时1:12小时。
				ptime:时间
**输出参数	:无
**返回值	: -2:非法参数 -1:24小时制,0:上午需加上AM 1:下午PM

*****************************************************************************/

int FrontOsd::CheckTimeByMode(unsigned char timemode,SystemDateTime *ptime)
{
	int retval = -2;
	if(ptime == NULL)
	{
		return retval;
	}
	if( timemode == 0 )
	{
		retval = -1;
	}
	else if( timemode == 1 )
	{
		if(ptime->hour == 0)
		{
			ptime->hour =12;
			retval = 0;//am
		}
		else if((ptime->hour >=1)&&(ptime->hour <=11))
		{
			retval = 0;//am
		}
		else if(ptime->hour == 12)
		{
			retval = 1;//pm
		}
			
		else if(ptime->hour > 12)
		{
			ptime->hour-=12;/*大于12小时且是12小时制则减去12 */
			retval = 1;//pm
		}

	}
	return retval;

}



