

#ifndef _JIANGHM_ZMD_MOBILE_USER_HEADER_3294878934279342849832
#define _JIANGHM_ZMD_MOBILE_USER_HEADER_3294878934279342849832

#include "tcplibdef.h"
#include "tcplib.h"
#include "owsp_def.h"

#define VGA_FRAME_BUF_SIZE		512*1024

class CMobileUser
{
public:
	CMobileUser() ;
	~CMobileUser() ;

public:

public:

    //初始化用户，将用户的信息全部置为默认值
    void ResetUser( ) ;

    //======================================
    //用户事件处理函数
    void onClose() ;
    bool onReceive( char *data , int len , int &used ) ;
    bool onIdle() ;

protected:

    bool onPacketProcesser( char *data , int len ) ;
    bool processerTlvPacket( char *data ,  int len ) ;

    //=============================================
    //消息处理函数

    bool onVersionInfoRequest( char *data , int len ) ;
    bool onPhoneInfoRequest( char *data , int len ) ;
    bool onLoginRequest( char *data , int len ) ;
    bool onChannelRequest( char *data , int len ) ;
    bool onControlRequest( char *data , int len ) ;

    //===============================================
    //回应函数


    //===============================================
    //功能处理函数
    int	RequestStartVideoLiving( int channel ) ;

    int SendLoginSuccess(  ) ;

    int SendVideoLiving( ) ;

    unsigned int GetSeq( )
    {
        return m_packetSeq ++ ;
    }

    unsigned int GetFrameCount( )
    {
        return m_frameCount ++ ;
    }

public:

    void			*m_sessionHandle ;
    int				m_mediaCmd ;		//记录用户请求媒体数据的信令号(CMD_START_VIDEO,CMD_START_SUBVIDEO,CMD_START_720P)
    int				m_Channel ;			//记录用户请求的通道id
    int				m_ChannelType ;		//记录用户请求的码流类型( VGA,QVGA,720P)

    //用户ID,全局为一个用户标识
    int				m_nUserID ;
    //用户申请的媒体通道id.
    int				m_mediaSid ;
    //用户申请的报警通道id
    int				m_alarmID ;


    //直播标志
    int				m_bStartSendMedia ;

    unsigned int	m_nSendPos ;				//上次发送的文件位置
    unsigned int	m_timeTick ;			//计时器

    unsigned int	m_remoteIP ;

    int				m_bUpdateFile ;

    char			m_FrameBuf[VGA_FRAME_BUF_SIZE] ;  

    TLV_V_VersionInfoRequest	m_VersionInfoRequest;

    unsigned int	m_packetSeq ;
    unsigned int	m_frameCount ;


};


#endif

