

#ifndef _JIANGHM_HTTP_SERVER_HEADER_32489327498327432
#define _JIANGHM_HTTP_SERVER_HEADER_32489327498327432

#include "tcplibdef.h"
#include <time.h>
#define GetHttpServer		CHttpServer::getInstance

#define MAX_HTTP_SIZE		4096

class CHttpServer
{
    //将构造申明为保护函数，防止外部的实例化
protected:
    CHttpServer() ;
    ~CHttpServer() ;

public:

    DECLARE_SINGLEOBJ( CHttpServer ) ;

public:
    //启动server
    bool	StartServer( char *addr , unsigned short port ) ;
    void	*m_serverHandle ;

protected:

    static bool onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr ) ;
    static void onServerTcpClose( int userData ) ;
    static bool onServerRecvData( int userData , char *data , int len , int &used ) ;
    static bool onServerSessionIdle( int userData ) ;

protected:

    //int		m_language ;

protected:

    bool onHttpRequest( void *session , char *data , int len ) ;
    bool onGetPage( void *session , char *page ) ;
    bool onPostPage( void *session , char *request , int len ) ;


    void	send404Page( void *session ) ;
    void	sendHttpResponseHeader( void *session , const char *retCode , const char *contentType ) ;
    void	getNowGmtTime( struct tm *gmt ) ;
    char	*gmtTimeToString( struct tm gmt , char *result ) ;
    const char	*getContentType( char *page ) ;

    char	*getContent( char *request , int len ) ;

    void	downString( char *szStr ) ;

    bool	getPostParam( const char *cmd , char split , char *src , char *result , int resultBufLen ) ;

    void	showControlPage( void *session , char *host , int port , int language , char *loginUser ) ;
    void	sendLoginFailedPage( void *session , int language ) ;
    void	sendVerifyVideoPortFaildPage( void *session , int language ); /*add by hayson 2014.1.17*/


};




#endif


