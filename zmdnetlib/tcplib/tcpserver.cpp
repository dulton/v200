
#include "tcplib.h"
#include "coreobj.h"

//--------------------------------------------
//服务器结构
typedef struct
{
    void					*server_handle ;		//服务器句柄
    int						server_sock ;			//服务器监听sock
    void					*accept_thread_handle ;	//服务器监听线程句柄
    onAcceptTcpLibCallback	accept_callback ;		//连接到达事件回调
    onCloseTcpLibCallback	onclose_callback ;		//连接关闭事件回调
    onReceiveTcpLibCallback	onrecv_callback ;		//连接数据到达事件回调
    onIdleTcpLibCallback	onidle_callback ;
} ZMD_SERVER_INFO ;


typedef struct
{
    int					session_sock ;
    int					userData ;
    unsigned int		remote_ip ;
    unsigned int		remote_port ;
    void				*server_handle ;
    void				*session_thread_handle ;	//会话线程句柄

    int					recv_buf_cursor ;		  	//接收缓冲区的
    int					run ;						//运行标志
    CMutex				send_mutex ;				//该锁用来保障数据包完整和互斥性

    onReceiveTcpLibCallback	recv_cb ;				//接收数据通知回调
    onCloseTcpLibCallback	close_cb ;				//关闭事件通知回调
    onIdleTcpLibCallback	idle_cb ;				//空闲通知
    char				recv_buf[TCP_RECV_BUF_SIZE] ;

} ZMD_SESSION_INFO ;


//==============================================================
//Tcp会话方法调用
/*
* Function:		创建一个tcp会话，用来与客户机通讯，

* Called By:
* Input:

	@serverHandle :	该会话所属的tcp服务器句柄
	@sock :			该会话用来通讯的socket句柄
	@recvcb :		该会话接收到数据通知
	@closecb :		该会话连接关闭通知
	@idlecb :		该会话空闲通知

* Output:
* Return:	会话句柄。返回NULL为创建失败

* Others:
*/
void *CreateTcpSession( void *serverHandle , int sock , onReceiveTcpLibCallback recvcb ,
                        onCloseTcpLibCallback closecb , onIdleTcpLibCallback idlecb );

/*
* Function:		销毁一个tcp会话,该接口只开放给tcplistener使用，外部禁止使用
				要在会话线程结束会话请调用StopTcpSession.
				由于C没有protected, friend概念，所以没办法保护该接口。

* Called By:
* Input:

	@handle :	会话句柄

* Output:
* Return:
* Others:
*/
void DestroyTcpSession( void *handle ) ;

/*
* Function:		启动会话，会自行创建线程，使用select模型
				当网络事件到达时，会自行调用recvcb和closecb,idlecb接口
				该接口只开放给tcplistener使用，外部禁止使用

* Called By:
* Input:

	@sessionHandle :	会话句柄
	@userData :			业务层使用索引

* Output:
* Return:			true:创建成功, false:创建失败
* Others:
*/
bool StartTcpSessionByThread( void *sessionHandle , int userData ) ;


bool OnTcpReceive( ZMD_SESSION_INFO *pSession  ) ;
void OnTcpClose( ZMD_SESSION_INFO *sessionHandle ) ;
bool OnTcpIdle( ZMD_SESSION_INFO *sessionHandle ) ;
//-----------------------------------
// 服务器会话处理线程
//------------------------------------
void *tcpSessionThreadProc( void *userData )
{
	TCP_PRINTF(" threadid %d \n", (unsigned)pthread_self());
	
	fd_set readfds;
	fd_set writefds;

    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)userData ;
    if( !pSession )
        return 0 ;
#ifdef DM368
	/**
	 * add by mike, 2013-11-21
	 * 当使用该TCP会话向NVR发送视频数据时，如果一帧
	 * 数据较大，而网络环境又不太理想，阻塞发送会
	 * 导致IPC丢帧，加大发送缓冲区可以较小阻塞发送
	 * 的时间。
	 */
	TcpSetSendBuff( pSession->session_sock, 100*1024 );
	#endif
    while( pSession->run )
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        FD_SET( pSession->session_sock , &readfds);
        int maxfd = pSession->session_sock ;

        timeval to;
        to.tv_sec = 0;
        to.tv_usec = 0 ;

        int n = select(maxfd + 1, &readfds, &writefds, NULL, &to);
        if( n > 0 )
        {
            if (FD_ISSET( pSession->session_sock , &readfds ))
            {
                //触发接收事件
                if( !OnTcpReceive( pSession ) )
                {
                    //业务处理出现错误，退出
                    RELEASE_CPU( 20 ) ;
                    break ;
                }
            }
        }
        if( OnTcpIdle( pSession ) )
            RELEASE_CPU( 1 ) ;
    }
    //通知上层会话结束
    OnTcpClose( pSession ) ;

    return 0 ;
}

//----------------------------------------------------------
//创建一个tcp会话，用来与客户机通讯
// @serverHanle :	服务器句柄
// @sock :			客户端sock
// @recvcb :		数据达到通知回调
// @closecb :		连接关闭通知回调
//-----------------------------------------------------------
void *CreateTcpSession( void *serverHandle , int sock,
                        onReceiveTcpLibCallback recvcb , onCloseTcpLibCallback closecb , onIdleTcpLibCallback idlecb )
{
    ZMD_SERVER_INFO *pServer = (ZMD_SERVER_INFO *)serverHandle ;
    //printf( "Before Malloc Session Info !\r\n" ) ;
    //创建一个会话
    ZMD_SESSION_INFO *pSession = new ZMD_SESSION_INFO ;//(ZMD_SESSION_INFO*)DebugMalloc( sizeof( ZMD_SESSION_INFO ) ) ;
    if( !pSession || !pServer )
    {
        return 0 ;
    }
    //printf( "Malloc Session Info !\r\n" ) ;

    //保存会话信息
    pSession->run = 1 ;
    pSession->server_handle = serverHandle ;
    pSession->session_sock = sock ;
    pSession->recv_buf_cursor = 0 ;
    pSession->recv_cb = recvcb ;
    pSession->close_cb = closecb ;
    pSession->idle_cb = idlecb ;

    return pSession ;
}

void DestroyTcpSession( void *handle )
{
    delete (ZMD_SESSION_INFO *)handle ;
}

bool StartTcpSessionByThread( void *sessionHandle , int userData )
{
    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
    if( !pSession )
        return false ;

    pSession->userData = userData ;

    //printf( "StartTcpSessionByThread !!\r\n" ) ;
    //启动数据处理线程
    pSession->session_thread_handle = CreateZmdThread( pSession , tcpSessionThreadProc ) ;

    //printf( "StartTcpSessionByThread successfully!!\r\n" ) ;

    if( !pSession->session_thread_handle )
    {
        //DebugFree( pSession ) ;
        return false ;
    }

    return true ;
}

//------------------------------------------------------
// 通知接收TCP数据
// @pSession :		会话句柄
//------------------------------------------------------
bool OnTcpReceive( ZMD_SESSION_INFO *pSession  )
{
    //检查buf

    if( pSession->recv_buf_cursor >= TCP_RECV_BUF_SIZE )
    {
        ERR_TCP("error!!! recv_buf_cursor >= TCP_RECV_BUF_SIZE !error , reset session!\r\n" ) ;
        return false ;
    }

    //调整接收缓冲区
    char *recvBuf = pSession->recv_buf + pSession->recv_buf_cursor ;
    int buflen = TCP_RECV_BUF_SIZE - pSession->recv_buf_cursor ;
    int ret = recv( pSession->session_sock , recvBuf , buflen , 0) ;

    if( ret > 0)
    {
        pSession->recv_buf_cursor += ret ;
        if( pSession->recv_buf_cursor > TCP_RECV_BUF_SIZE )
        {
            ERR_TCP( "error!! recvbufcursor > TCP_RECV_BUF_SIZE !!\r\n" ) ;
            return false ;
        }

        int used = 0 ;
        //----------------------------------------------
        //数据提交上层处理，必须返回拆包后所使用的字节数
        //程序会将未使用数据保存下来继续接收数据
        if( !pSession->recv_cb( pSession->userData , pSession->recv_buf , pSession->recv_buf_cursor , used ) )
        {
            return false ;
        }

        //检查used.
        if( used < 0 || used >= TCP_RECV_BUF_SIZE )
        {
            ERR_TCP( "tcpclient used error %d !!!\r\n" , used ) ;
            return false ;
        }

        pSession->recv_buf_cursor -= used ;

        //将未处理完的数据保存下来，继续接收数据直到凑成一个完成包
        if( pSession->recv_buf_cursor > 0 )
        {
            memmove( pSession->recv_buf , pSession->recv_buf + used , pSession->recv_buf_cursor ) ;
        }
    }
    else
    {

#ifdef _WIN32
        uint32 ercode = GetLastError() ;
        if( MYEWOULDBLOCK != ercode )
        {
			printf( "(%d)recv error sock = %d ercode = %d !!\r\n" , __LINE__,pSession->session_sock , ercode ) ;
#else
        if( MYEWOULDBLOCK != errno )
        {
			//printf( "(%d)recv error sock = %d ercode = %d !!\r\n" ,__LINE__, pSession->session_sock , errno ) ;
#endif
            return false ;
        }
    }

    return true ;
}


int TcpBlockRecv( void *sessionHandle , char *buf , int buflen , int timeout )
{
    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
    if( !pSession )
        return 0 ;

    int nLeft = buflen ;
    int nRecved = 0 ;
    int nRet = 0 ;

    TCP_PRINTF( "TcpBlockRecv()!!!! buflen = %d , timeout=%d\r\n" , buflen , timeout) ;

    fd_set readfds;
    while( nLeft > 0 )
    {
        FD_ZERO(&readfds);

        FD_SET( pSession->session_sock , &readfds);
        int maxfd = pSession->session_sock + 1;

        timeval to;
        to.tv_sec = timeout / 1000 ;
        to.tv_usec = (timeout % 1000) * 1000 ;

        int n = select( maxfd , &readfds, NULL , NULL, &to);
        //timeout,直接返回已接收到的数据量
        if( n <= 0)
        {
			perror("##### select\r\n");
            return nRecved ;
        }

        //由于只有一个fd在fdset里，不判断FD_ISSET了
        nRet = recv( pSession->session_sock , buf + nRecved , nLeft , 0 ) ;

        //接收出错
        if( nRet <= 0 )
        {
            ERR_TCP( "recv failed!!!\r\n" ) ;
            return nRecved ;
        }

        nRecved += nRet ;
        nLeft -= nRet ;
    }

    TCP_PRINTF( "recv complete nLeft = %d , nRecved = %d \r\n" , nLeft , nRecved ) ;

    return nRecved ;
}

int GetSessionSock(void *sessionHandle)
{
	ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
	return pSession->session_sock;
}

//---------------------------------------------------------
// 阻塞发送一块数据
// @sessionHandle : 会话句柄
// @data : 数据指针
// @datalen : 数据长度
//--------------------------------------------------
int TcpBlockSend( void *sessionHandle , char *data , int datalen , int timeout )						
{
    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
    if( !pSession ) 										
    {
        ERR_TCP("TcpBlockSend() pSession is NULL!!\r\n" ) ;
        return 0 ;
	}
	
    //该锁保证每个数据的发送的完整性
    CAutoMutex atlck( &(pSession->send_mutex ));

    int nLeft = datalen ;
    int nSend = 0 ;
    char *pCursor = data ;

    while( nLeft )
    {
		
		fd_set writefds;

        FD_ZERO(&writefds);

        FD_SET( pSession->session_sock , &writefds);
        int maxfd = pSession->session_sock ;

        timeval to;
        to.tv_sec = timeout;
        to.tv_usec = 0 ;

        int n = select(maxfd + 1, NULL, &writefds, NULL, &to);
        if( n > 0 )
        {
            if (FD_ISSET( pSession->session_sock , &writefds ))
            {
                nSend = ::send( pSession->session_sock , (char *)pCursor , nLeft , 0 ) ;
                //printf("send ret = %d\r\n" , nSend ) ;
            }
        }

        //nSend = ::send( pSession->session_sock , (char*)pCursor , nLeft , 0 ) ;

        if( nSend <= 0 )
        {

            /*
            #ifdef _WIN32
            			uint32 ercode = GetLastError() ;
            			if( MYEWOULDBLOCK != ercode ){
            #else
            			if( errno !=  MYEWOULDBLOCK ){
            #endif
            	*/
            //socket错误 退出
            ERR_TCP( "TcpBlockSend send failed!!!!! errcode = %d\r\n" , errno ) ;
            pSession->run = false ;
            return 0 ;
//			}
//			continue ;
        }
        nLeft -= nSend ;
        pCursor += nSend ;
    }

    return (datalen - nLeft) ;
}

//---------------------------------------
//通知上层，会话关闭
void OnTcpClose( ZMD_SESSION_INFO *sessionHandle )
{
    if( !sessionHandle )
        return ;

    sessionHandle->close_cb( sessionHandle->userData ) ;
    void *sessionThread = sessionHandle->session_thread_handle ;

    //关闭会话sock
    if( -1 != sessionHandle->session_sock )
    {
        mysocketclose( sessionHandle->session_sock ) ;
        //printf( "close socket %d \r\n" , sessionHandle->session_sock ) ;
        sessionHandle->session_sock = -1 ;
    }

    //释放会话
    //DebugFree( sessionHandle ) ;
    delete sessionHandle ;

    return ;

}

bool OnTcpIdle( ZMD_SESSION_INFO *sessionHandle )
{
    if( !sessionHandle )
        return true ;

    return sessionHandle->idle_cb( sessionHandle->userData ) ;
}

//-----------------------------------
// 温和关闭会话
void StopTcpSession( void *sessionHandle )
{
    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
    if( !pSession )
        return ;

    pSession->run = 0 ;
}

//-------------------------------------
//强制手段，立即终止会话
void TerminateTcpSession( void *sessionHandle )
{
    ZMD_SESSION_INFO *pSession = (ZMD_SESSION_INFO *)sessionHandle ;
    if( !pSession )
        return ;

    pSession->run = 1 ;

    void *sessionThread = pSession->session_thread_handle ;
    //关闭句柄
    if( -1 != pSession->session_sock )
    {
        mysocketclose( pSession->session_sock ) ;
        //printf( "close socket %d \r\n" , pSession->session_sock ) ;
        pSession->session_sock = -1 ;
    }

    //释放会话对象
    //DebugFree( pSession ) ;
    delete pSession ;

    //从外部终止线程
    if( sessionThread )
    {
        DestroyThread( sessionThread ) ;
    }
}

void *listenThreadProc( void *obj )
{
	ZMD_SERVER_INFO* pInfo = (ZMD_SERVER_INFO*)obj ;
	TCP_PRINTF(" threadid %d \r\n", (unsigned)pthread_self());

    while( true )
    {
        struct sockaddr_in clientaddr ;
#ifdef _WIN32
        int clilen = sizeof( clientaddr ) ;
#else
        socklen_t clilen = sizeof( clientaddr ) ;
#endif

        int connectfd = accept( pInfo->server_sock , (struct sockaddr *)&clientaddr , &clilen ) ;
        if( connectfd < 0 )
        {
#ifdef _WIN32
            printf( "accept error!\r\n" ) ;
#else
            ERR_TCP( "accept error! errno = %d \r\n" , errno ) ;
#endif
            RELEASE_CPU( 20 ) ;
            continue ;
        }

        //printf( "accept session!!\r\n" ) ;
        //处理accept的链接
        int userData = -1 ;

#ifndef _WIN32
        //int set = 1;
        //setsockopt( connectfd , SOL_SOCKET, SO_NOSIGPIPE, (void*)&set, sizeof(int));
#endif

        void *session = CreateTcpSession( pInfo , connectfd , pInfo->onrecv_callback , pInfo->onclose_callback , pInfo->onidle_callback )  ;

        //printf( "CreateTcpSession!!\r\n" ) ;

        if( pInfo->accept_callback( userData , session , &clientaddr ) )
        {
            StartTcpSessionByThread( session , userData ) ;
        }
        else
        {
            //delete session ;
            DestroyTcpSession( session ) ;
            mysocketclose( connectfd ) ;

        }


    }
    return 0 ;
}

// 创建Tcp服务器,
// 返回服务器句柄.
// 0为创建失败
void *CreateTcpServer( onAcceptTcpLibCallback acceptcb , onCloseTcpLibCallback onclosecb ,
                       onReceiveTcpLibCallback onrecvcb , onIdleTcpLibCallback idlecb)
{
    ZMD_SERVER_INFO *pInfo = (ZMD_SERVER_INFO *)DebugMalloc( sizeof( ZMD_SERVER_INFO ) ) ;
    if( !pInfo )
        return 0 ;

    pInfo->server_handle = (void *)pInfo ;
    pInfo->accept_callback = acceptcb ;
    pInfo->onclose_callback = onclosecb ;
    pInfo->onrecv_callback = onrecvcb ;
    pInfo->onidle_callback = idlecb ;

    return pInfo->server_handle ;
}

void DestroyTcpServer( int serverHandle )
{

    return ;
}

void StopTcpServer( int serverHandle )
{

}

//-----------------------------------------------
// 启动服务器
// @serverHandle : 服务器句柄
// @addr [in] : 服务器监听地址
// @port [out] : 服务器端口
// @isBlock : 是否需要重复尝试绑定，直到成功
//-----------------------------------------------
bool StartTcpServer( void *serverHandle , char *addr , unsigned short port , bool isBlock )
{
    ZMD_SERVER_INFO *pInfo = (ZMD_SERVER_INFO *)serverHandle ;
    if( !pInfo )
        return false ;

    int listensock = socket( AF_INET , SOCK_STREAM , 0 ) ;

    //创建句柄失败
    if( -1 == listensock )
    {
        ERR_TCP( "socket() failed !\r\n" ) ;
        return false ;
    }

    //设置可重用
    int on = 1;
    if ( setsockopt( listensock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on) ) < 0 )
    {
        ERR_TCP( "setsockopt SO_REUSEADDR failed !\r\n" ) ;
    }

    //fork 的时候, 子进程不继承网络监听句柄
    int flags = fcntl(listensock, F_GETFD); 
    flags |= FD_CLOEXEC; 
    fcntl(listensock, F_SETFD, flags);

    while( true )
    {
        struct sockaddr_in bindaddr ;
        bindaddr.sin_family = AF_INET ;
        bindaddr.sin_port = htons( port ) ;
#ifndef _WIN32
        inet_aton( addr , &(bindaddr.sin_addr ) ) ;
#else
        bindaddr.sin_addr.S_un.S_addr = inet_addr( addr ) ;
#endif

        if( bind( listensock , (struct sockaddr *)&bindaddr , sizeof( bindaddr ) ))
        {
            ERR_TCP( "bind error: %s:%d ercode=%d sock=%d\r\n" , addr , port , errno , listensock ) ;

            //不需要重复尝试，直接返回失败
            if( !isBlock )
            {
				mysocketclose( listensock ) ;
				return false ;
			}
			RELEASE_CPU( 5000 ) ;
            continue ;
        }
        break ;
    }

    //开始监听
    if( listen( listensock , 5 ) )
    {
		perror( "listen error : " ) ;
		mysocketclose( listensock ) ;
		return false ;
	}

    //保存监听的句柄
    pInfo->server_sock = listensock ;

    //启动监听线程
    pInfo->accept_thread_handle = CreateZmdThread( pInfo , listenThreadProc ) ;

    return true ;
}

#ifdef DM368
//设置TCP 发送缓冲区大小
static void TcpSetSendBuff( int fd, int len )
{
	int retOpt;
	int nSndBuf = len;
	
	if( fd <= 0 || len <= 0 )
		return;

	retOpt = setsockopt( fd, SOL_SOCKET, SO_SNDBUF, (const char *) &nSndBuf, sizeof(int) );
	if( retOpt )
	{
		return;
	}
	
	//printf("TcpSetSendBuff: set send buff %d success!\n", len);
}

#endif


