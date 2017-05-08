
//+---------------------------------------------------------------------------
//
//  File:   	mobileserver.h
//
//  Author:		jianghm
//
//  Contents:   mobileserver服务器实现
//
//  Notes:
//
//  Version:	1.0
//
//  Date:		2012-12-11
//
//  History:
// 			 jianghm	2012-12-11   1.0	   创建文件
//----------------------------------------------------------------------------


#ifndef _JIANGHM_MOBILE_SERVER_HEADER_2349832423423
#define _JIANGHM_MOBILE_SERVER_HEADER_2349832423423

#include "tcplibdef.h"
#include "coreobj.h"


#define GetMobileServerObj		CMobileServer::getInstance

class CMobileUser ;

typedef struct
{
    CMobileUser	*userobj ;		//播放用户,此处从堆中分配对象
    int				userid ;		//播放用户id .
    int				mediaid ;		//记录用户在media队列中的索引号
    int				alarmid ;		//记录用户在alarm队列中的索引号

    int				used ;			//是否使用
} STRUCT_MOBILE_USER ;

//////////////////////////////////////////////////////////////////////
//Mobile协议服务器，结构与CNetServer类似。信令处理不同
class CMobileServer
{
    //将构造申明为保护函数，防止外部的实例化
protected:
    CMobileServer() ;
    ~CMobileServer() ;

public:

    DECLARE_SINGLEOBJ( CMobileServer )

public:
    //启动server
    bool		StartServer( char *addr , unsigned short port ) ;

    //获取空闲的工作会话
    int							getUnuseWorkSession( ) ;
    //释放工作会话
    void						freeWorkSession( int userid ) ;


protected:

    static bool onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr ) ;
    static void onServerTcpClose( int userData ) ;
    static bool onServerRecvData( int userData , char *data , int len , int &used ) ;
    static bool onServerSessionIdle( int userData ) ;

    //==============================================
    //事件处理函数
    void onClose( int userData ) ;
    bool onReceive( int userData , char *data , int len , int &used ) ;
    bool onIdle( int userData ) ;
    bool onAccept( int &userData , void *session , sockaddr_in *client_addr ) ;

protected:
    // tcp工作会话管理
    STRUCT_MOBILE_USER			m_loginUserList[MAX_NET_TCP_USER] ;
    CMutex						m_loginUserListMutex ;

    void						*m_serverHandle ;

    //记载已连接的tcp数量
    int							m_nUserCount ;
};


#endif


