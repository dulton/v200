
//+---------------------------------------------------------------------------
//
//
//
//  File:   	tcpclient.cpp
//
//  Author:		jianghm
//
//  Contents:   tcp客户端功能封装
//
//  Notes:
//
//  Version:	1.0
//
//  Date:		2012-12-13
//
//  History:
// 			jianghm	2012-12-13	1.0		创建文件
//			jianghm 2012-12-20	1.1		modify
//	@修改接口，新增ResetClient接口
//	@修正tcpclient断开重连后，没有重置recv_buf_cursor.造成接收数据不正确的bug.
//----------------------------------------------------------------------------

#include "tcplibdef.h"
#include "tcplib.h"
#include "coreobj.h"

typedef struct
{
    int					client_sock ;
    struct sockaddr_in	remote_addr ;			//远程地址
    int					connected ;				//socket句柄
    int					userData ;				//用来通知上层回调时的用户数据
    int					isReconnect ;

    void				*client_thread_handle ;	//客户线程句柄

    int					recv_buf_cursor ;		//接收缓冲区的
    int					run ;					//运行标志
    CMutex				send_mutex ;			//该锁用来保障数据包完整和互斥性

    onConnectTcpLibCallback		connect_callback ;		//连接通知
    onCloseTcpLibCallback		close_callback;			//接收数据通知
    onReceiveTcpLibCallback		receive_callback ;		//关闭事件通知回调
    onIdleTcpLibCallback		idle_callback ;			//空闲通知

    char				recv_buf[TCP_RECV_BUF_SIZE] ;	//tcp接收缓冲区

} ZMD_CLIENT_INFO ;

bool ClientConnect( ZMD_CLIENT_INFO *pClient ) ;
bool SetNoneblocking( int sock ) ;
bool OnClientReceive( ZMD_CLIENT_INFO *pClient  ) ;
void OnClientClose( ZMD_CLIENT_INFO *clientHandle ) ;

bool SelectRecv( int sock )
{
    fd_set readfds;

    FD_ZERO(&readfds);

    FD_SET( sock , &readfds );
    int maxfd = sock + 1;

    timeval to;
    to.tv_sec = 0;
    to.tv_usec = 0 ;

    int n = select(maxfd , &readfds, NULL, NULL, &to);

    if( n > 0 )
    {
        if (FD_ISSET( sock , &readfds ))
        {
            return true ;
        }
    }
    return false ;
}

//------------------------------------
// 客户端会话处理线程
//------------------------------------
void *tcpClientThreadProc( void *userData )
{
	TCP_PRINTF("threadid %d \r\n", (unsigned)pthread_self());
	
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)userData ;
    if( !pClient )
        return 0 ;

    //--------------------------
    //线程中会一直尝试连接远程地址
    //当连接成功后会响应OnConnect事件
    while( pClient->run )
    {
        if( !pClient->connected )
        {
            //连接断开的状态，就尝试连接
            if( !ClientConnect( pClient ) )
            {
                unsigned char *remoteip = (unsigned char *) & (pClient->remote_addr.sin_addr) ;

                //printf( "connect failed.%d.%d.%d.%d..retry after 5 seconds .\r\n" , remoteip[0] , remoteip[1] , remoteip[2] , remoteip[3] ) ;
                //是否需要重连
                if( pClient->isReconnect )
                {
                    RELEASE_CPU( 3000 ) ;
                    continue ;
                }
                else
                {
                    break ;
                }
            }
        }
        else
        {

            if( SelectRecv( pClient->client_sock ) )
            {
                //已经连接则处理数据
                if( !OnClientReceive( pClient ) )
                {
                    //数据处理错误，通知业务层关闭
                    OnClientClose( pClient ) ;
                    pClient->connected = 0 ;

                    //准备重连
                    unsigned char *remoteip = (unsigned char *) & (pClient->remote_addr.sin_addr) ;
                    //printf( "disconnect %d.%d.%d.%d..retry after 3 seconds .\r\n" , remoteip[0] , remoteip[1] , remoteip[2] , remoteip[3] ) ;

                    //是否需要重连
                    if( pClient->isReconnect )
                    {
                        RELEASE_CPU( 3000 ) ;
                        continue ;
                    }
                    else
                    {
                        break ;
                    }
                }
            }


            pClient->idle_callback( pClient->userData ) ;
            RELEASE_CPU( 20 ) ;
        }
    }

    //关闭会话sock句柄
    if( -1 != pClient->client_sock )
    {
        mysocketclose( pClient->client_sock ) ;
        //printf( "client close socket %d \r\n" , pClient->client_sock ) ;
        pClient->client_sock = -1 ;
    }

    TCP_PRINTF( "*********** Client Session stop!!***********\r\n" ) ;

    //销毁客户端对象
    delete pClient ;

    return 0 ;
}

bool ClientConnect( ZMD_CLIENT_INFO *pClient )
{
    if( !pClient )
    {
        return false ;
    }

    //关闭前期句柄
    if( -1 != pClient->client_sock )
    {
        mysocketclose( pClient->client_sock ) ;
        pClient->client_sock = -1 ;
    }

    //创建句柄
    pClient->client_sock = socket( AF_INET , SOCK_STREAM , 0 ) ;

    if( -1 == pClient->client_sock )
    {
        return false ;
    }

    pClient->connected = 0 ;

    unsigned char *remoteip = (unsigned char *) & (pClient->remote_addr.sin_addr) ;

    //printf( "connect %d.%d.%d.%d...\r\n" , remoteip[0] , remoteip[1] , remoteip[2] , remoteip[3] ) ;


    //尝试连接
    if (::connect( pClient->client_sock , (sockaddr *) &pClient->remote_addr, sizeof(sockaddr)) != 0)
    {
        mysocketclose( pClient->client_sock ) ;
        pClient->client_sock = -1 ;
        pClient->connect_callback( pClient->userData , pClient->connected );
        return false ;
    }

    pClient->connected = 1 ;
    pClient->recv_buf_cursor = 0 ;
    memset( pClient->recv_buf , 0 , sizeof( pClient->recv_buf ) ) ;
    //SetNoneblocking( pClient->client_sock ) ;
    pClient->connect_callback( pClient->userData , pClient->connected );

    return true ;
}

bool SetNoneblocking( int sock )
{
#ifdef _WIN32
    unsigned long ul = 1;
    if( ioctlsocket(sock, FIONBIO, &ul) == SOCKET_ERROR )
    {
        return false ;
    }

    //设置连接的心跳检查
    /*
    tcp_keepalive TcpLive = { 0 };
    tcp_keepalive OutTcpLive = { 0 };
    DWORD dwBytes ;

    TcpLive.onoff = 1;
    TcpLive.keepalivetime = 15000;
    TcpLive.keepaliveinterval = 3000;
    if (SOCKET_ERROR == ::WSAIoctl(	sock,
    	SIO_KEEPALIVE_VALS,
    	&TcpLive,
    	sizeof(struct tcp_keepalive),
    	&OutTcpLive,
    	sizeof(struct tcp_keepalive),
    	&dwBytes,
    	NULL,
    	NULL ))
    {
    	return true ;
    }
    */

    return true ;
#else

    /*
    int keepAlive = 1;		// Open keepalive
    int keepIdle = 15;		// 如该连接在30秒内没有任何数据往来,则进行探测
    int keepInterval = 5; // 探测时发包的时间间隔为5 秒
    int keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
    */

    int ops ;
    ops = fcntl( sock , F_GETFL ) ;
    if( ops < 0 )
    {
        return false ;
    }

    ops |= O_NONBLOCK ;
    if( fcntl( sock , F_SETFL , ops ) < 0 )
    {
        return false ;
    }

    return true ;
#endif
}

//----------------------------------------------------------
//创建一个tcp会话，用来与客户机通讯
// @serverHanle :	服务器句柄
// @sock :			客户端sock
// @recvcb :		数据达到通知回调
// @closecb :		连接关闭通知回调
//-----------------------------------------------------------
void *CreateClient(struct sockaddr_in *addr , int userData , int isReconnect ,
                   onConnectTcpLibCallback connect_cb , onCloseTcpLibCallback close_cb ,
                   onReceiveTcpLibCallback receive_cb , onIdleTcpLibCallback idle_cb )
{
    //创建一个客户端
    ZMD_CLIENT_INFO *pClient = new ZMD_CLIENT_INFO ; //(ZMD_CLIENT_INFO*)DebugMalloc( sizeof( ZMD_CLIENT_INFO ) ) ;
    if( !pClient )
    {
        return 0 ;
    }

    //保存会话信息
    //连接地址
    memcpy( &pClient->remote_addr , addr , sizeof( struct sockaddr_in ) ) ;

    //线程运行状态
    pClient->run = 1 ;
    pClient->connected = 0 ;
    pClient->client_sock = -1 ;
    pClient->recv_buf_cursor = 0 ;
    pClient->userData = userData ;

    //设置回调
    pClient->connect_callback = connect_cb ;
    pClient->close_callback = close_cb ;
    pClient->receive_callback = receive_cb ;
    pClient->idle_callback = idle_cb ;

    pClient->isReconnect = isReconnect ;

    //启动数据处理线程
    pClient->client_thread_handle = CreateZmdThread( pClient , tcpClientThreadProc ) ;
    if( !pClient->client_thread_handle )
    {
        DebugFree( pClient ) ;
        return 0 ;
    }

    return pClient ;

}

//------------------------------------------------------
// 发现socket句柄上有读事件后，接收TCP数据
// @pSession :		会话句柄
//------------------------------------------------------
bool OnClientReceive( ZMD_CLIENT_INFO *pClient  )
{
    //检查buf
    if( pClient->recv_buf_cursor >= TCP_RECV_BUF_SIZE )
    {
        ERR_TCP("%d!!! recv_buf_cursor >= TCP_RECV_BUF_SIZE !error , reset session!\r\n" , pClient->recv_buf_cursor ) ;
        return false ;
    }

    //如果存在剩余数据，调整接收缓冲区
    char *recvBuf = pClient->recv_buf + pClient->recv_buf_cursor ;
    int buflen = TCP_RECV_BUF_SIZE - pClient->recv_buf_cursor ;
    int ret = recv( pClient->client_sock , recvBuf , buflen , 0) ;

    if( ret > 0)
    {
        pClient->recv_buf_cursor += ret ;
        if( pClient->recv_buf_cursor > TCP_RECV_BUF_SIZE )
        {
            ERR_TCP( "error!! recvbufcursor > TCP_RECV_BUF_SIZE !!\r\n" ) ;
            return false ;
        }

        int used = 0 ;
        //----------------------------------------------
        //数据提交上层处理，必须返回拆包后所使用的字节数
        //程序会将未使用数据保存下来继续接收数据
        if( !pClient->receive_callback( pClient->userData , pClient->recv_buf , pClient->recv_buf_cursor , used ) )
        {
            return false ;
        }

        //检查used.
        if( used < 0 || used >= TCP_RECV_BUF_SIZE )
        {
            ERR_TCP( "tcpclient used error %d !!!\r\n" , used ) ;
            return false ;
        }

        pClient->recv_buf_cursor -= used ;

        //将未处理完的数据保存下来，继续接收数据直到凑成一个完成包
        if( pClient->recv_buf_cursor > 0 )
        {
            memmove( pClient->recv_buf , pClient->recv_buf + used , pClient->recv_buf_cursor ) ;
        }
    }
    else
    {

#ifdef _WIN32
        uint32 ercode = GetLastError() ;
        if( MYEWOULDBLOCK != ercode )
#else
        if( MYEWOULDBLOCK != errno )
#endif
        {
            //printf( "recv error sock = %d ercode = %d !!\r\n" , pClient->client_sock , errno ) ;
            //printf( "recv error sock = %d ercode = %d !!\r\n" , pClient->client_sock , ercode ) ;
            return false ;
        }
    }

    return true ;
}

bool SelectSend( int sock )
{
    fd_set writefds;

    FD_ZERO(&writefds);

    FD_SET( sock , &writefds );
    int maxfd = sock + 1;

    timeval to;
    to.tv_sec = 5;
    to.tv_usec = 0 ;

    int n = select(maxfd , NULL, &writefds, NULL, &to);

    if( n > 0 )
    {
        if (FD_ISSET( sock , &writefds ))
        {
            return true ;
        }
    }
    return false ;
}

//---------------------------------------------------------
// 阻塞发送一块数据
// @sessionHandle : 会话句柄
// @data : 数据指针
// @datalen : 数据长度
//--------------------------------------------------
int ClientBlockSend( void *clientHandle , char *data , int datalen )
{
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)clientHandle ;
    if( !pClient )
        return 0 ;

    //该锁保证每个数据包的发送的完整性
    CAutoMutex atlck( &(pClient->send_mutex ));

    int nLeft = datalen ;
    int nSend = 0 ;
    char *pCursor = data ;

    while( nLeft && pClient->run )
    {
        //查询是否可写
        if( !SelectSend( pClient->client_sock ) )
            return 0 ;

        nSend = ::send( pClient->client_sock , (char *)pCursor , nLeft , 0 ) ;
        if( nSend <= 0 )
        {
#ifdef _WIN32
            uint32 ercode = GetLastError() ;
            if( MYEWOULDBLOCK != ercode )
#else
            if( errno !=  MYEWOULDBLOCK )
#endif
            {
                //socket错误 重置连接标识，返回错误
                pClient->connected = 0 ;
                return 0 ;
            }
            continue ;
        }
        nLeft -= nSend ;
        pCursor += nSend ;
    }

    return (datalen - nLeft) ;
}

//--------------------------------------------
// 阻塞接收数据
// 接收指定大小buf的数据，填满缓冲或者出现错误才会返回
bool ClientBlockRecv( void *clientHandle , char *buf , int *len )
{
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)clientHandle ;
    if( !pClient )
        return 0 ;

    //缓冲大小
    int nBufLen = *len ;
    int nRecv = 0 ;
    int nLeft = nBufLen ;

    while( nLeft > 0 )
    {
        int nRet = recv( pClient->client_sock , buf + nRecv , nLeft , 0 ) ;
        if( 0 == nRet )
        {
            //连接已关闭,接收失败,设置连接失败
            pClient->connected = 0 ;
            return false ;
        }

        if( nRet < 0 )
        {
#ifdef _WIN32
            uint32 ercode = GetLastError() ;
            if( MYEWOULDBLOCK != ercode )
#else
            if( errno !=  MYEWOULDBLOCK )
#endif
            {
                //socket错误 退出，设置连接失败
                pClient->connected = 0 ;
                return false ;
            }
        }

        nRecv += nRet ;
        nLeft -= nRet ;
    }
    return true ;
}

//---------------------------------------
//通知上层，会话关闭
void OnClientClose( ZMD_CLIENT_INFO *clientHandle )
{
    if( !clientHandle )
        return ;

    //关闭句柄
    if( -1 != clientHandle->client_sock )
    {
        mysocketclose( clientHandle->client_sock ) ;
        clientHandle->client_sock = -1 ;
    }

    clientHandle->recv_buf_cursor = 0 ;

    clientHandle->close_callback( clientHandle->userData  ) ;
    return ;
}

//-----------------------------------
// 温和关闭会话
void StopClient( void *clientHandle )
{
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)clientHandle ;
    if( !pClient )
        return ;

    pClient->run = 0 ;
}

//-------------------------------------
//强制手段，立即终止会话
void TerminateClient( void *clientHandle )
{
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)clientHandle ;
    if( !pClient )
        return ;

    pClient->run = 1 ;

    void *sessionThread = pClient->client_thread_handle ;

    //关闭句柄
    if( -1 != pClient->client_sock )
    {
        mysocketclose( pClient->client_sock ) ;
        TCP_PRINTF( "close socket %d \r\n" , pClient->client_sock ) ;
        pClient->client_sock = -1 ;
    }

    //释放会话对象
    //DebugFree( pClient ) ;
    delete pClient ;

    //从外部终止线程
    if( sessionThread )
    {
        DestroyThread( sessionThread ) ;
    }
}

void ResetClient( void *clientHandle , struct sockaddr_in *addr )
{
    ZMD_CLIENT_INFO *pClient = (ZMD_CLIENT_INFO *)clientHandle ;

    if( !pClient )
        return ;

    if( addr )
    {
        memcpy( &pClient->remote_addr , addr , sizeof( struct sockaddr_in ) ) ;
    }

    OnClientClose( pClient ) ;
    pClient->connected = 0 ;

}



