

#ifndef _JIANGHM_MEDIA_SESSION_MANAGER_HEADER_32439876432234
#define _JIANGHM_MEDIA_SESSION_MANAGER_HEADER_32439876432234

#include "tcplibdef.h"
#include "coreobj.h"

#define GetMediaMgr		CMediaSessionMgr::getInstance

typedef struct
{
    int		used[MAX_MEDIA_PLAYER] ;
} STRUCT_MEDIA_SESSION ;

class CMediaSessionMgr
{
protected:
    CMediaSessionMgr() ;
    ~CMediaSessionMgr() ;

public:

    DECLARE_SINGLEOBJ( CMediaSessionMgr ) ;

    //获取空闲的媒体会话
    int							getUnuseMediaSession( int channel , int type ) ;
    //释放媒体会话
    void						freeMediaSession( int channel , int type , int mediaid ) ;

protected:

    int							getUnuseVgaSession( int channel ) ;
    int							getUnuseQvgaSession( int channel ) ;
    int							getUnuse720PSession( int channel ) ;

    int							getUnuseRecordSession( int channel ) ;


    void						freeVgaSession( int channel , int mediaid ) ;
    void						freeQvgaSession( int channel , int mediaid ) ;
    void						free720PSession( int channel , int mediaid ) ;

    void						freeRecordSession( int channel , int mediaid ) ;

protected:

    //=========================================
    //是分别为720p,vga,qvga建立3个会话队列
    STRUCT_MEDIA_SESSION		m_vgaSessionList[IPC_CLIENT_COUNT] ;
    CMutex						m_vgaSessionListMutex ;

    //=========================================
    STRUCT_MEDIA_SESSION		m_qvgaSessionList[IPC_CLIENT_COUNT] ;
    CMutex						m_qvgaSessionListMutex ;

    //=========================================
    STRUCT_MEDIA_SESSION		m_p720SessionList[IPC_CLIENT_COUNT] ;
    CMutex						m_p720SessionListMutex ;

    //=========================================
    //录像播放和下载队列
    STRUCT_MEDIA_SESSION		m_recordSessionList[IPC_CLIENT_COUNT] ;
    CMutex						m_recordSessionListMutex ;

    //记载目前申请的媒体通道数量
    int							m_mediaChnCount ;
    CMutex						m_mediaChntMutex ;

};


#endif






