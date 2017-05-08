#include "tcplibdef.h"
#include "mediamgr.h"

IMPLEMENT_SINGLEOBJ( CMediaSessionMgr )

CMediaSessionMgr::CMediaSessionMgr()
{
    m_mediaChnCount = 0 ;
    //初始化medialist
    memset( &m_vgaSessionList , 0 , sizeof( m_vgaSessionList ) ) ;
    memset( &m_qvgaSessionList , 0 , sizeof( m_vgaSessionList ) ) ;
    memset( &m_p720SessionList , 0 , sizeof( m_vgaSessionList ) ) ;
    memset( &m_recordSessionList , 0 , sizeof( m_vgaSessionList ) ) ;
}

CMediaSessionMgr::~CMediaSessionMgr()
{

}

int CMediaSessionMgr::getUnuseMediaSession( int channel , int type )
{
    if( channel < 0 || channel >= IPC_CLIENT_COUNT )
    {
        printf( "CMediaSessionMgr::getUnuseMediaSession() error channle = %d\r\n" , channel ) ;
        return INVALID_SESSION ;
    }

    int mediaSessionID = INVALID_SESSION ;
    switch( type )
    {
    case VGA_CHN_TYPE :
        mediaSessionID = getUnuseVgaSession( channel ) ;
        break ;
    case QVGA_CHN_TYPE :
        mediaSessionID = getUnuseQvgaSession( channel ) ;
        break ;
    case D720P_CHN_TYPE :
        mediaSessionID = getUnuse720PSession( channel ) ;
        break ;
    case ALARM_CHN_TYPE :
        break ;
    case RECORD_CHN_TYPE :
        //录像统一只是用一个通道
        mediaSessionID = getUnuseRecordSession( 0 ) ;
        break ;
    default :
        break ;
    }

    if( INVALID_SESSION != mediaSessionID )
    {
        CAutoMutex atlck( &m_mediaChntMutex ) ;
        m_mediaChnCount ++ ;
    }

    return mediaSessionID ;
}

void CMediaSessionMgr::freeMediaSession( int channel , int type , int mediaid )
{
    if( channel < 0 || channel >= IPC_CLIENT_COUNT )
    {
        printf( "CMediaSessionMgr::freeMediaSession() error channle = %d\r\n" , channel ) ;
        return  ;
    }

    switch( type )
    {
    case VGA_CHN_TYPE :
        freeVgaSession( channel , mediaid ) ;
        break ;
    case QVGA_CHN_TYPE :
        freeQvgaSession( channel , mediaid ) ;
        break ;
    case D720P_CHN_TYPE :
        free720PSession( channel , mediaid ) ;
        break ;
    case ALARM_CHN_TYPE :
        break ;
    case RECORD_CHN_TYPE :
        freeRecordSession( channel , mediaid ) ;
        break ;
    default :
        break ;
    }

    m_mediaChntMutex.lock() ;
    m_mediaChnCount ++ ;
    m_mediaChntMutex.unlock() ;

}

int	CMediaSessionMgr::getUnuseVgaSession( int channel )
{
    CAutoMutex atlck( &m_vgaSessionListMutex ) ;
    int i = 0 ;
    //由于bufmanager认为0为非法的。。所以从1开始
    for( i = 1 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( !m_vgaSessionList[channel].used[i] )
        {
            //获取到可用媒体会话
            m_vgaSessionList[channel].used[i] = 1 ;
            return i ;
        }
    }

    return INVALID_SESSION ;
}

int	CMediaSessionMgr::getUnuseQvgaSession( int channel )
{
    CAutoMutex atlck( &m_qvgaSessionListMutex ) ;

    int i = 0 ;
    //由于bufmanager认为0为非法的。。所以从1开始
    for( i = 1 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( !m_qvgaSessionList[channel].used[i] )
        {
            //获取到可用媒体会话
            m_qvgaSessionList[channel].used[i] = 1 ;
            return i ;
        }
    }

    return INVALID_SESSION ;

}

int	CMediaSessionMgr::getUnuse720PSession( int channel )
{
    CAutoMutex atlck( &m_p720SessionListMutex ) ;

    int i = 0 ;
    //由于bufmanager认为0为非法的。。所以从1开始
    for( i = 1 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( !m_p720SessionList[channel].used[i] )
        {
            //获取到可用媒体会话
            m_p720SessionList[channel].used[i] = 1 ;
            return i ;
        }
    }

    return INVALID_SESSION ;

}

int	CMediaSessionMgr::getUnuseRecordSession( int channel )
{
    CAutoMutex atlck( &m_recordSessionListMutex ) ;

    int i = 0 ;
    for( i = 0 ; i < MAX_MEDIA_PLAYER ; i ++ )
    {
        if( !m_recordSessionList[channel].used[i] )
        {
            //获取到可用媒体会话
            m_recordSessionList[channel].used[i] = 1 ;
            return i ;
        }
    }

    return INVALID_SESSION ;

}

void CMediaSessionMgr::freeVgaSession( int channel , int mediaid )
{
    if( mediaid < 0 || mediaid >= MAX_MEDIA_PLAYER )
    {
        printf( "freeVgaSession() error mediaid = %d \r\n" , mediaid ) ;
        return ;
    }

    CAutoMutex atlck( &m_vgaSessionListMutex ) ;
    m_vgaSessionList[channel].used[mediaid] = 0 ;
}

void CMediaSessionMgr::freeQvgaSession( int channel , int mediaid )
{
    if( mediaid < 0 || mediaid >= MAX_MEDIA_PLAYER )
    {
        printf( "freeQvgaSession() error mediaid = %d \r\n" , mediaid ) ;
        return ;
    }

    CAutoMutex atlck( &m_qvgaSessionListMutex ) ;
    m_qvgaSessionList[channel].used[mediaid] = 0 ;

}

void CMediaSessionMgr::free720PSession( int channel , int mediaid )
{
    if( mediaid < 0 || mediaid >= MAX_MEDIA_PLAYER )
    {
        printf( "free720PSession() error mediaid = %d \r\n" , mediaid ) ;
        return ;
    }

    CAutoMutex atlck( &m_p720SessionListMutex ) ;
    m_p720SessionList[channel].used[mediaid] = 0 ;

}

void CMediaSessionMgr::freeRecordSession( int channel , int mediaid )
{
    if( mediaid < 0 || mediaid >= MAX_MEDIA_PLAYER )
    {
        printf( "freeRecordSession() error mediaid = %d \r\n" , mediaid ) ;
        return ;
    }

    CAutoMutex atlck( &m_recordSessionListMutex ) ;
    m_recordSessionList[channel].used[mediaid] = 0 ;

}







