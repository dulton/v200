
#include "mobileserver.h"
#include "mobileuser.h"
#include "tcplib.h"

IMPLEMENT_SINGLEOBJ( CMobileServer )

CMobileServer::CMobileServer()
{
    m_nUserCount = 0 ;
}

CMobileServer::~CMobileServer()
{

}

//==============================================
//启动服务器
bool CMobileServer::StartServer( char *addr , unsigned short port )
{
	//初始化工作会话
	int i = 0 ;
    for( i = 0 ; i < MAX_NET_TCP_USER ; i ++ )
    {
        m_loginUserList[i].userobj = new CMobileUser() ;
        m_loginUserList[i].userid = i ;
        m_loginUserList[i].used = 0 ;
        m_loginUserList[i].mediaid = INVALID_USERID ;
        m_loginUserList[i].alarmid = INVALID_USERID ;
    }

    //创建一个tcp服务器
    m_serverHandle = CreateTcpServer( onServerTcpAccept , onServerTcpClose , onServerRecvData , onServerSessionIdle ) ;
    if( m_serverHandle )
    {
        //启动该tcp服务器
        if( StartTcpServer( m_serverHandle , addr , port , true ))
        {
            return true ;
        }
    }
    return false ;
}

bool CMobileServer::onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr )
{
    //通知有链接到达
    return getInstance()->onAccept( userData , session , client_addr ) ;
}

void CMobileServer::onServerTcpClose( int userData )
{
    //通知有链接关闭
    getInstance()->onClose( userData ) ;
}

bool CMobileServer::onServerSessionIdle( int userData )
{
    //通知有链接关闭
    return getInstance()->onIdle( userData ) ;
}

//--------------------------------------------
//对接收到的数据进行分包
bool CMobileServer::onServerRecvData( int userData , char *data , int len , int &used )
{
    return getInstance()->onReceive( userData , data , len , used ) ;
}

//=================================================
//获取一个空闲的用户结构.
int	CMobileServer::getUnuseWorkSession( )
{
    //进入工作会话锁
    CAutoMutex atlck( &m_loginUserListMutex ) ;

    int i = 0 ;
    for( i = 0 ; i < MAX_NET_TCP_USER ; i ++ )
    {
        if( !m_loginUserList[i].used )
        {
            m_loginUserList[i].used = 1 ;
            return i ;
        }
    }

    return INVALID_USERID ;
}

//=============================================
//释放用户结构
void CMobileServer::freeWorkSession( int userid )
{
    if( userid < 0 || userid >= MAX_NET_TCP_USER )
    {
        printf( "freeWorkSession() userid error = %d\r\n" , userid ) ;
        return ;
    }

    CAutoMutex atlck( &m_loginUserListMutex ) ;
    m_loginUserList[userid].used = 0 ;
}

void CMobileServer::onClose( int userData )
{
    //通知有链接关闭
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return ;

    m_nUserCount -- ;
    printf( "CMobileServer::onClose usercount = %d \r\n" , m_nUserCount ) ;

    m_loginUserList[userid].userobj->onClose() ;
}

bool CMobileServer::onReceive( int userData , char *data , int len , int &used )
{
    printf( "CMobileServer::onReceive\r\n" ) ;
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return false ;

    return m_loginUserList[userid].userobj->onReceive( data , len , used ) ;
}

bool CMobileServer::onIdle( int userData )
{
    //通知空闲
    int userid = userData ;

    if( userid < 0 || userid >= MAX_NET_TCP_USER )
        return true ;

    return m_loginUserList[userid].userobj->onIdle() ;
}

bool CMobileServer::onAccept( int &userData , void *session , sockaddr_in *client_addr )
{
    //通知有链接到达
    //获取工作会话(工作会话有最大限制.)
    int userid = getUnuseWorkSession() ;
    if( INVALID_USERID == userid )
    {
        return false ;
    }

    m_nUserCount ++ ;
    printf( "CMobileServer::onAccep usercount = %d \r\n" , m_nUserCount ) ;

    //获取到可用工作会话后，对会话进行初始化
    //由于不会被外部线程关闭，所以这里可以访问
    //m_loginUserList[userid].userobj.而不需要加锁

    userData = userid ;
    m_loginUserList[userid].userobj->ResetUser() ;
    m_loginUserList[userid].userobj->m_sessionHandle = session ;
    m_loginUserList[userid].userobj->m_nUserID = userid ;
    m_loginUserList[userid].userobj->m_remoteIP = client_addr->sin_addr.s_addr ;

    return true ;
}


