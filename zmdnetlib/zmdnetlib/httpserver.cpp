#include <string>

#include "httpserver.h"
#include "netserver.h"
#include "tcplib.h"
#include "md5.h"


#define HTTP_ROOT		"/app/dvr/"
//#define MAX_REC_CHANNEL	8
#include "ModuleFuncInterface.h"


#define HTTP_RESP_TEXT		\
"HTTP/1.1 %s\r\n\
Content-Type: %s\r\n\
Server: httpserver\r\n\
Data: %s\r\n\
Last Modified: %s\r\n\r\n"

#define SHOW_CONTROL_PAGE	\
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n\
	<html xmlns=\"http://www.w3.org/1999/xhtml\">\n\
	<head>\n\
	<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n\
	<title>IP Camera Manager</title>\n\
	<style type=\"text/css\">\n\
	<!--\n\
	body {\n\
	\tmargin-left: 0px;\n\
	\tmargin-top: 0px;\n\
	\tmargin-right: 0px;\n\
	\tmargin-bottom: 0px;\n\
	\tbackground-color: #63D2FF;\n\
	}\n\
	-->\n\
	</style>\n\
	<script type=\"text/javascript\">\n\
	function InitControl()\n\
	{\n\
	\tZmodo_ActiveX.SetHostInfo(\"%s\",%d,%d);\n\
	\tZmodo_ActiveX.SetSysLanguage(%d);\n\
	\tZmodo_ActiveX.SetLoginUser(\"%s\");\n\
	}\n\
	</script>\n\
	</head>\n\
	<body onload=\"InitControl()\">\n\
	<table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n\
	<tbody><tr>\n\
	<td align=\"center\"><table width=\"100%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr>\n\
	<td align=\"center\"><table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n\
	<tr>\n\
	<td>\n\
	<object classid=\"CLSID:D9305048-DD6B-4EDF-8706-096EBE24E1D7\" codebase=\"IPCWeb.cab#version=%s\"   width=\"1000\", height=\"620\" id=\"Zmodo_ActiveX\" onclick=\"window.location.reload(true);\"></object>\n\
	</td>\n\
	</tr>\n\
	</table></td>\n\
	</tr></table></td>\n\
	</tr></tbody>\n\
	</table>\n\
	</body>\n\
	</html>\n"



IMPLEMENT_SINGLEOBJ( CHttpServer )

CHttpServer::CHttpServer()
{
	
}


CHttpServer::~CHttpServer()
{
	
}

///----------------------------------------------
//启动服务器
bool CHttpServer::StartServer( char *addr , unsigned short port )
{
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

bool CHttpServer::onServerTcpAccept( int &userData , void *session , sockaddr_in *client_addr )
{
    //通知有链接到达
    //printf( "onTcpAccept!!\r\n" ) ;

    userData = (int)session ;
    return true ;
}

void CHttpServer::onServerTcpClose( int userData )
{
    //通知有链接关闭
    //printf( "onTcpClose!!\r\n" ) ;
}

bool CHttpServer::onServerSessionIdle( int userData )
{
    //通知有链接关闭
    return true ;
}

//--------------------------------------------
//对接收到的数据进行分包
bool CHttpServer::onServerRecvData( int userData , char *data , int len , int &used )
{
    used = len ;

    return GetHttpServer()->onHttpRequest( (void *)userData , data , len ) ;
}

#ifdef _WIN32
char *strsep(char **s, const char *ct)
{
    char *sbegin = *s;
    char *end;

    if (sbegin == NULL)
        return NULL;

    end = strpbrk(sbegin, ct);
    if (end)
        *end++ = '\0';
    *s = end;
    return sbegin;
}


#endif

bool CHttpServer::onHttpRequest( void *session , char *data , int len )
{
    //最后要预留一个\0字符，所以-1
    if( len > MAX_HTTP_SIZE - 1 )
        return false ;

    char	reqBuf[MAX_HTTP_SIZE] ;

    memcpy( reqBuf , data , len ) ;
    reqBuf[len] = 0 ;
    data[len] = 0 ;

    //printf( "%s\r\n" , reqBuf ) ;

    char *cur = reqBuf ;

    char *method = strsep(&cur, " ");
    char *path = strsep(&cur, " ");
    char *request_page = 0 ;
    char *request_string = 0 ;

    if( !method || !path )
        return false ;

    if(strcmp(method, "GET") == 0)
    {
        request_page = strsep(&path, " ?");
        if(path == NULL)
            request_string = NULL;
        else
            request_string = strsep(&path, " ");

        //printf("get page:%s string:%s\n",request_page,request_string);

        if(strcmp(request_page, "/") == 0)
        {
            request_page = (char *)"/index.htm";
        }

        onGetPage( session , request_page ) ;

    }
    else if(strcmp(method, "POST") == 0)
    {
        request_page = strsep(&path, " ?");
        request_string = path;
        //printf("post page:%s string:%s\n",request_page,request_string);
        //这里传入data，因为reqBuf中数据已经破坏
        //downString( data );
        onPostPage( session , data , len );
    }

    return false ;
}

static std::string get_file_name (const int fd)  
{  
    if (0 >= fd) {  
        return std::string ();  
    }  
  
    char buf[1024] = {'\0'};  
    char file_path[1024] = {'0'}; // PATH_MAX in limits.h  
    snprintf(buf, sizeof (buf), "/proc/self/fd/%d", fd);  
    if (readlink(buf, file_path, sizeof(file_path) - 1) != -1) {  
        return std::string (file_path);  
    }  
  
    return std::string ();  
}  

//=========================================
//处理GET请求
bool CHttpServer::onGetPage( void *session , char *page )
{
    char szFilePathName[1024] ;

    snprintf(szFilePathName ,sizeof(szFilePathName), "%s%s" , HTTP_ROOT , page ) ;

    FILE *file = fopen( szFilePathName , "rb" ) ;
    if( !file )
    {
        send404Page( session ) ;
        return false ;
    }

	//安全检查
	std::string fname = get_file_name(fileno(file));
	printf("onGetPage:[%s]\n", fname.c_str());

	if(strstr(fname.c_str(), HTTP_ROOT) == NULL)
	{
		printf("get a file not in %s is forbidden!!!\n", HTTP_ROOT);
		fclose(file);
		send404Page( session ) ;
		return false ;		
	}
    //从后缀获取content type
    char contentType[128] ;
    strcpy( contentType , getContentType( page ) ) ;

    sendHttpResponseHeader( session , "200 OK" , contentType ) ;

    fseek( file , 0 , SEEK_SET ) ;
    char filebuf[4096] ;
    int nRead = 0 ;
    while(true)
    {
        nRead = fread( filebuf , 1, 4096 , file ) ;
        int nSend = TcpBlockSend( session , filebuf , nRead ) ;
        //发送失败,关闭退出
        if( nSend != nRead )
        {
            fclose( file ) ;
            return false ;
        }

        if( nRead != 4096 )
            break ;
    }

    fclose( file ) ;

    return false ;
}

//===================================================
//发送404页面
void CHttpServer::send404Page( void *session )
{

}

//===================================================
//发送http请求回应
void CHttpServer::sendHttpResponseHeader( void *session , const char *retCode , const char *contentType )
{
    char szTimeGmt[128] ;
    struct tm gmt ;
    getNowGmtTime( &gmt ) ;
    gmtTimeToString( gmt , szTimeGmt ) ;

    char respHeader[1024] ;

    sprintf( respHeader , HTTP_RESP_TEXT , retCode , contentType , szTimeGmt , szTimeGmt ) ;

    //printf( respHeader ) ;

    int nSend = TcpBlockSend( session , respHeader , strlen( respHeader ) ) ;

}

//==================================================
//取现在时间
void CHttpServer::getNowGmtTime( struct tm *gmt )
{
    time_t now = time(NULL) ;
    struct tm *gmtnow = localtime( &now ) ;

    memcpy( gmt , gmtnow , sizeof( tm ) ) ;

    gmt->tm_year += 1900 ;
    gmt->tm_mon += 1 ;
}

static const char day_name[7][4] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char mon_name[12][4] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

//---------------------------------
//将GMT时间转换成Mon, 28 Nov 2011 09:58:44 GMT
//格式字符串时间
char *CHttpServer::gmtTimeToString( struct tm gmt , char *result )
{
    //Date: Mon, 28 Nov 2011 09:58:44 GMT
    sprintf(result, "%.3s, %.2d %.3s %.4d %.2d:%.2d:%.2d GMT",
            day_name[gmt.tm_wday] , gmt.tm_mday , mon_name[gmt.tm_mon - 1] ,
            gmt.tm_year , gmt.tm_hour , gmt.tm_min , gmt.tm_sec );

    return result;
}

//===================================================
//获取http数据头中的contentType
const char *CHttpServer::getContentType( char *page )
{
    //static char contentType[256][5] = { "text/html"

    if( strstr( page , ".htm" ) != NULL || strstr( page , ".html" ) != NULL )
    {
        return "text/html" ;
    }

    if( strstr( page , ".jpg" ) != NULL || strstr( page , ".gif" ) != NULL )
    {
        return "image/jpeg" ;
    }

    if( strstr( page , ".gif" ) != NULL )
    {
        return "image/gif" ;
    }

    if( strstr( page , ".cab" ) != NULL )
    {
        return "application/cab" ;
    }

    return "application/octet-stream" ;

}

//=====================================================
//处理post请求
bool CHttpServer::onPostPage( void *session , char *request , int len )
{
    char szHost[64] = {0};

    //http请求中的主机字段
    getPostParam( "Host: " , '\r' , request , szHost , 64 ) ;

    //从host中提取ip
    int i = 0 ;
    for( i = 0 ; i < strlen( szHost ) ; i ++ )
    {
        if( '\r' == szHost[i] || '\n' == szHost[i] || ':' == szHost[i] )
        {
            szHost[i] = 0 ;
            break ;
        }
    }


    //最后一行是参数
    char param[260] ;
    char *content = getContent( request , len ) ;
    if( !content || strlen( content ) > 260 - 1 )
        return false ;

    strcpy( param , content ) ;

    //提取参数中的内容
    char username[128] ;
    char password[128] ;
    char strPort[128] ;
    char strLanguage[128] ;
    char userReal[32] ;

    //提出参数内容
    if( !getPostParam( "UserName=" , '&' , param , username , 128 ) )
        return false ;

    if( !getPostParam( "PassWord=" , '&' , param , password , 128 ) )
        return false ;

    if( !getPostParam( "Port=" , '&' , param , strPort , 128 ) )
    {
       // strcpy( strPort , "8000" ) ;
       return false;
    }

    if( !getPostParam( "language=" , '&' , param , strLanguage , 128 ) )
    {
        strcpy( strLanguage , "1" ) ;
    }

    int port = atoi( strPort ) ;
    int language = atoi( strLanguage ) ;
	
	if(GetNetServerObj()->VerifyVideoPort(port) != 0)
	{
		sendVerifyVideoPortFaildPage( session , language ) ;
		return false;
	}

    //验证用户
    int permit ;
    int echo = GetNetServerObj()->VerifyUserByMd5( username , password , permit , (char *)userReal ) ;
    if( !echo )
    {
        showControlPage( session , szHost , port , language , userReal ) ;
        return true ;
    }

    sendLoginFailedPage( session , language ) ;

    return false ;

}

//================================================
//发送登陆失败页面
void CHttpServer::sendLoginFailedPage( void *session , int language )
{
    //发送http header .
    sendHttpResponseHeader( session , "200 OK" , "text/html" ) ;

    char line[1024] ;
    char page[32 * 1024] ;

    memset( page , 0 , sizeof( page ) ) ;

    sprintf( line, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
    strcat( page , line ) ;
    sprintf( line, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    strcat( page , line ) ;
    sprintf( line, "<head>\n");
    strcat( page , line ) ;
    sprintf( line, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
    strcat( page , line ) ;

    if(language)
    {
        sprintf( line, "<title>UserName Or PassWord Error !</title>\n");
    }
    else
    {
        sprintf( line, "<title>用户名或密码错误 !</title>\n");
    }
    strcat( page , line ) ;

    sprintf( line, "<style type=\"text/css\">\n");
    strcat( page , line ) ;
    sprintf( line, "<!--\n");
    strcat( page , line ) ;

    sprintf( line, "body {\n");
    strcat( page , line ) ;
    sprintf( line, "margin:0px auto;\n");
    strcat( page , line ) ;
    sprintf( line, "padding:0px;\n");
    strcat( page , line ) ;
    sprintf( line, "text-align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "background-color: #FFFFFF;\n");
    strcat( page , line ) ;
    sprintf( line, "background:url(./img/bj.jpg) repeat-x;\n");
    strcat( page , line ) ;
    sprintf( line, "text-align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "width:100%%; \n");
    strcat( page , line ) ;
    sprintf( line, "height:100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "/*filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(src='./img/bj.jpg',sizingMethod='scale');\n");
    strcat( page , line ) ;
    sprintf( line, "background-repeat:no-repeat;\n");
    strcat( page , line ) ;
    sprintf( line, "background-positon:100%%,100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "font:normal 12px tahoma,arial,verdana,sans-serif;\n");
    strcat( page , line ) ;
    sprintf( line, "margin:0px auto;\n");
    strcat( page , line ) ;
    sprintf( line, "padding:0px;\n");
    strcat( page , line ) ;
    sprintf( line, "border:0 none;\n");
    strcat( page , line ) ;
    sprintf( line, "overflow:hidden;\n");
    strcat( page , line ) ;
    sprintf( line, "width:100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "height: 100%%;*/\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, "*{ padding:0; margin:0;}\n");
    strcat( page , line ) ;
    sprintf( line, ".td{ padding-left:0px;}\n");
    strcat( page , line ) ;
    sprintf( line, "/*.user_login { border: 1px solid #607A92; color: #36618B; height: 24px; line-height: 24px; width: 160px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".password_login { border: 1px solid #0073A9; color: #BDCEDE; height: 24px; line-height: 24px; width: 160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".port_login { border: 1px solid #0073A9; color: #BDCEDE; height: 24px; line-height: 24px; width: 160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}*/\n");
    strcat( page , line ) ;
    sprintf( line, ".user_login { height:24px; line-height:24px; width:160px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".password_login { height:24px; line-height:24px; width:160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".port_login { height:24px; line-height:24px; width:160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, "#mid{ 	\n");
    strcat( page , line ) ;
    sprintf( line, "width:444px; \n");
    strcat( page , line ) ;
    sprintf( line, "height:275px; \n");
    strcat( page , line ) ;
    sprintf( line, "margin:auto;\n");
    strcat( page , line ) ;
    sprintf( line, "margin-top:5%%;\n");
    strcat( page , line ) ;
    sprintf( line, "line-height:35px;\n");
    strcat( page , line ) ;
    sprintf( line, "margin-left:auto;\n");
    strcat( page , line ) ;
    sprintf( line, "align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, ".STYLE1 {color: #FFFFFF}\n");
    strcat( page , line ) ;
    sprintf( line, "-->\n");
    strcat( page , line ) ;
    sprintf( line, "</style>\n");
    strcat( page , line ) ;
    sprintf( line, "</head>\n");
    strcat( page , line ) ;
    sprintf( line, "<body>\n");
    strcat( page , line ) ;
    sprintf( line, "<br />\n");
    strcat( page , line ) ;
    sprintf( line, "<br />\n");
    strcat( page , line ) ;
    sprintf( line, "<form action=\"showControl\" method=\"post\" name=\"Login\" target=\"_self\" id=\"Login\">\n");
    strcat( page , line ) ;
    sprintf( line, "<div id=\"mid\" style=\"background-image:url(./img/bg.jpg); background-repeat:no-repeat; padding:1px 0;\">\n");
    strcat( page , line ) ;
    sprintf( line, "<p align=\"center\" style=\"margin:10px 0 0 15px; padding:0; height:35px;\"></p>\n");
    strcat( page , line ) ;
    sprintf( line, "<table class=\"\" width=\"\" height=\"\" border=\"0\" align=\"center\" style=\"padding:0; margin:30px auto 0 auto; width:80%%;\">\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"81\" height=\"22\"><div align=\"right\"><span class=\"STYLE1\">UserName:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"text\" maxlength=\"16\" name=\"UserName\" width=\"200px\" height=\"30px\" size=\"28\" valign=\"middle\" class=\"password_login\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\" />\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"81\" height=\"24\"><div align=\"right\"><span class=\"STYLE1\">Password:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"password\" maxlength=\"16\" name=\"PassWord\" width=\"200px\" size=\"28\"  height=\"30px\" valign=\"middle\" class=\"password_login\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\"/>\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"><input name=\"ok\" type=\"image\" value=\"Submit\"  src=\"./img/ok.jpg\" width=\"50\" height=\"50\" align=\"center\" /></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td height=\"36\"><div align=\"right\"><span class=\"STYLE1\">VideoPort:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"text\" maxlength=\"5\" name=\"Port\" value=\"8000\" width=\"100px\" size=\"28\"  height=\"30px\" align=\"left\" class=\"port_login\" valign=\"middle\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\"/>\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\">&nbsp;</td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"radio\" name=\"language\" value=\"0\" />\n");
    strcat( page , line ) ;
    sprintf( line, "<img src=\"./img/chinese.jpg\" width=\"45\" height=\"11\" /> </span><a href=\"./IPCWeb.cab\"></a>\n");
    strcat( page , line ) ;
    sprintf( line, "<div align=\"center\"><a href=\"./IPCWeb.cab\"></a></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"170\"><div align=\"center\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"radio\" name=\"language\" value=\"1\" checked=\"checked\" />\n");
    strcat( page , line ) ;
    sprintf( line, "<img src=\"./img/english.jpg\" alt=\"language\" width=\"40\" height=\"10\" /></span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"><div align=\"left\"><a href=\"./IPCWeb.cab\"><img src=\"./img/dl.jpg\" alt=\"downLoad Video Tool\" width=\"71\" height=\"24\" border=\"0\" /></a></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "</table>\n");
    strcat( page , line ) ;
    sprintf( line, "</p>\n");
    strcat( page , line ) ;
    sprintf( line, "</div>\n");
    strcat( page , line ) ;
    sprintf( line, "</form>\n");
    strcat( page , line ) ;
    sprintf( line, "<script type=\"text/javascript\">\n");
    strcat( page , line ) ;
    sprintf( line, "window.onload = function(){\n");
    strcat( page , line ) ;
    if(language)
    {
        sprintf( line, "alert(\"Username Or Password Error !\");\n");
    }
    else
    {
		sprintf( line, "alert(\"用户名或密码错误 !\");\n");
    }
    strcat( page , line ) ;
    sprintf( line, "window.location.href=\"\\index.htm\";\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, "</script>\n");
    strcat( page , line ) ;

    sprintf( line, "</body>\n");
    strcat( page , line ) ;
    sprintf( line, "</html>\n");
    strcat( page , line ) ;

    TcpBlockSend( session , page , strlen( page ) ) ;

    return ;
}


//================================================
//发送登陆失败页面
void CHttpServer::sendVerifyVideoPortFaildPage( void *session , int language )
{
    //发送http header .
    sendHttpResponseHeader( session , "200 OK" , "text/html" ) ;

    char line[1024] ;
    char page[32 * 1024] ;

    memset( page , 0 , sizeof( page ) ) ;

    sprintf( line, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
    strcat( page , line ) ;
    sprintf( line, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    strcat( page , line ) ;
    sprintf( line, "<head>\n");
    strcat( page , line ) ;
    sprintf( line, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\n");
    strcat( page , line ) ;

    if(language)
    {
        sprintf( line, "<title>Video Port Error !</title>\n");
    }
    else
    {
        sprintf( line, "<title>视频端口错误 !</title>\n");
    }
    strcat( page , line ) ;

    sprintf( line, "<style type=\"text/css\">\n");
    strcat( page , line ) ;
    sprintf( line, "<!--\n");
    strcat( page , line ) ;

    sprintf( line, "body {\n");
    strcat( page , line ) ;
    sprintf( line, "margin:0px auto;\n");
    strcat( page , line ) ;
    sprintf( line, "padding:0px;\n");
    strcat( page , line ) ;
    sprintf( line, "text-align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "background-color: #FFFFFF;\n");
    strcat( page , line ) ;
    sprintf( line, "background:url(./img/bj.jpg) repeat-x;\n");
    strcat( page , line ) ;
    sprintf( line, "text-align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "width:100%%; \n");
    strcat( page , line ) ;
    sprintf( line, "height:100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "/*filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(src='./img/bj.jpg',sizingMethod='scale');\n");
    strcat( page , line ) ;
    sprintf( line, "background-repeat:no-repeat;\n");
    strcat( page , line ) ;
    sprintf( line, "background-positon:100%%,100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "font:normal 12px tahoma,arial,verdana,sans-serif;\n");
    strcat( page , line ) ;
    sprintf( line, "margin:0px auto;\n");
    strcat( page , line ) ;
    sprintf( line, "padding:0px;\n");
    strcat( page , line ) ;
    sprintf( line, "border:0 none;\n");
    strcat( page , line ) ;
    sprintf( line, "overflow:hidden;\n");
    strcat( page , line ) ;
    sprintf( line, "width:100%%;\n");
    strcat( page , line ) ;
    sprintf( line, "height: 100%%;*/\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, "*{ padding:0; margin:0;}\n");
    strcat( page , line ) ;
    sprintf( line, ".td{ padding-left:0px;}\n");
    strcat( page , line ) ;
    sprintf( line, "/*.user_login { border: 1px solid #607A92; color: #36618B; height: 24px; line-height: 24px; width: 160px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".password_login { border: 1px solid #0073A9; color: #BDCEDE; height: 24px; line-height: 24px; width: 160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".port_login { border: 1px solid #0073A9; color: #BDCEDE; height: 24px; line-height: 24px; width: 160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}*/\n");
    strcat( page , line ) ;
    sprintf( line, ".user_login { height:24px; line-height:24px; width:160px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".password_login { height:24px; line-height:24px; width:160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, ".port_login { height:24px; line-height:24px; width:160px; margin-left:0px; padding-left:0px; margin-bottom:10px;}\n");
    strcat( page , line ) ;
    sprintf( line, "#mid{ 	\n");
    strcat( page , line ) ;
    sprintf( line, "width:444px; \n");
    strcat( page , line ) ;
    sprintf( line, "height:275px; \n");
    strcat( page , line ) ;
    sprintf( line, "margin:auto;\n");
    strcat( page , line ) ;
    sprintf( line, "margin-top:5%%;\n");
    strcat( page , line ) ;
    sprintf( line, "line-height:35px;\n");
    strcat( page , line ) ;
    sprintf( line, "margin-left:auto;\n");
    strcat( page , line ) ;
    sprintf( line, "align:center;\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, ".STYLE1 {color: #FFFFFF}\n");
    strcat( page , line ) ;
    sprintf( line, "-->\n");
    strcat( page , line ) ;
    sprintf( line, "</style>\n");
    strcat( page , line ) ;
    sprintf( line, "</head>\n");
    strcat( page , line ) ;
    sprintf( line, "<body>\n");
    strcat( page , line ) ;
    sprintf( line, "<br />\n");
    strcat( page , line ) ;
    sprintf( line, "<br />\n");
    strcat( page , line ) ;
    sprintf( line, "<form action=\"showControl\" method=\"post\" name=\"Login\" target=\"_self\" id=\"Login\">\n");
    strcat( page , line ) ;
    sprintf( line, "<div id=\"mid\" style=\"background-image:url(./img/bg.jpg); background-repeat:no-repeat; padding:1px 0;\">\n");
    strcat( page , line ) ;
    sprintf( line, "<p align=\"center\" style=\"margin:10px 0 0 15px; padding:0; height:35px;\"></p>\n");
    strcat( page , line ) ;
    sprintf( line, "<table class=\"\" width=\"\" height=\"\" border=\"0\" align=\"center\" style=\"padding:0; margin:30px auto 0 auto; width:80%%;\">\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"81\" height=\"22\"><div align=\"right\"><span class=\"STYLE1\">UserName:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"text\" maxlength=\"16\" name=\"UserName\" width=\"200px\" height=\"30px\" size=\"28\" valign=\"middle\" class=\"password_login\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\" />\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"81\" height=\"24\"><div align=\"right\"><span class=\"STYLE1\">Password:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"password\" maxlength=\"16\" name=\"PassWord\" width=\"200px\" size=\"28\"  height=\"30px\" valign=\"middle\" class=\"password_login\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\"/>\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"><input name=\"ok\" type=\"image\" value=\"Submit\"  src=\"./img/ok.jpg\" width=\"50\" height=\"50\" align=\"center\" /></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td height=\"36\"><div align=\"right\"><span class=\"STYLE1\">VideoPort:</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><div align=\"left\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"text\" maxlength=\"5\" name=\"Port\" value=\"8000\" width=\"100px\" size=\"28\"  height=\"30px\" align=\"left\" class=\"port_login\" valign=\"middle\" style=\"background:url(./img/input.jpg); background-repeat:no-repeat;\"/>\n");
    strcat( page , line ) ;
    sprintf( line, "</span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\">&nbsp;</td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<tr>\n");
    strcat( page , line ) ;
    sprintf( line, "<td colspan=\"2\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"radio\" name=\"language\" value=\"0\" />\n");
    strcat( page , line ) ;
    sprintf( line, "<img src=\"./img/chinese.jpg\" width=\"45\" height=\"11\" /> </span><a href=\"./IPCWeb.cab\"></a>\n");
    strcat( page , line ) ;
    sprintf( line, "<div align=\"center\"><a href=\"./IPCWeb.cab\"></a></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"170\"><div align=\"center\"><span class=\"STYLE3\">\n");
    strcat( page , line ) ;
    sprintf( line, "<input type=\"radio\" name=\"language\" value=\"1\" checked=\"checked\" />\n");
    strcat( page , line ) ;
    sprintf( line, "<img src=\"./img/english.jpg\" alt=\"language\" width=\"40\" height=\"10\" /></span></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "<td width=\"126\"><div align=\"left\"><a href=\"./IPCWeb.cab\"><img src=\"./img/dl.jpg\" alt=\"downLoad Video Tool\" width=\"71\" height=\"24\" border=\"0\" /></a></div></td>\n");
    strcat( page , line ) ;
    sprintf( line, "</tr>\n");
    strcat( page , line ) ;
    sprintf( line, "</table>\n");
    strcat( page , line ) ;
    sprintf( line, "</p>\n");
    strcat( page , line ) ;
    sprintf( line, "</div>\n");
    strcat( page , line ) ;
    sprintf( line, "</form>\n");
    strcat( page , line ) ;
    sprintf( line, "<script type=\"text/javascript\">\n");
    strcat( page , line ) ;
    sprintf( line, "window.onload = function(){\n");
    strcat( page , line ) ;
    if(language)
    {
        sprintf( line, "alert(\"Video Port Error !\");\n");
    }
    else
    {
		sprintf( line, "alert(\"视频端口错误 !\");\n");
    }
    strcat( page , line ) ;
    sprintf( line, "window.location.href=\"\\index.htm\";\n");
    strcat( page , line ) ;
    sprintf( line, "}\n");
    strcat( page , line ) ;
    sprintf( line, "</script>\n");
    strcat( page , line ) ;

    sprintf( line, "</body>\n");
    strcat( page , line ) ;
    sprintf( line, "</html>\n");
    strcat( page , line ) ;

    TcpBlockSend( session , page , strlen( page ) ) ;

    return ;
}

//===================================
//取http表单中的content内容
char *CHttpServer::getContent( char *request , int len  )
{
    int reqSize = len ;

    //寻找2个连续的回车换行
    char *content = strstr( request , "\r\n\r\n" ) ;
    //验证找到地址是否非法.
    if( !content || content + 4 > request + len )
        return 0 ;

    return content + 4 ;
}

//=============================================
//将字符串转为小写
void CHttpServer::downString( char *szStr )
{
    char *pCursor = szStr ;
    while( *pCursor )
    {
        if(*pCursor >= 'A' && *pCursor <= 'Z' )
            *pCursor += 'a' - 'A';
        pCursor ++ ;
    }
}

//=================================================
//获取一个字符串中指定关键字后的数据内容
// cmd: 关键字
// split: 结束字符
// src: 源字符串
// result: 用来存储获取到的字符串的缓冲
// resultBufLen : 缓冲长度
bool CHttpServer::getPostParam( const char *cmd , char split , char *src , char *result , int resultBufLen )
{
    char *start = strstr( src , cmd ) ;
    if( !start )
        return false ;

    start += strlen( cmd ) ;

    char *pCursor = start ;

    //找到命令字，随后开始扫描后面字串，直到找到split
    while( pCursor < src + strlen( src ) )
    {
        if( split == *pCursor )
        {
            int paramLen = pCursor - start ;
            if( paramLen > resultBufLen )
                paramLen = resultBufLen ;

            memcpy( result , start , paramLen ) ;
            result[paramLen] = 0 ;
            return true ;
        }
        pCursor ++ ;
    }

    //至字符串尾部，则start就是目标串
    strncpy( result , start , resultBufLen ) ;
    result[resultBufLen - 1] = 0 ;

    return true ;
}

//==================================================
//显示IE控件页面
void CHttpServer::showControlPage( void *session , char *host , int port , int language  , char *loginUser )
{
    int peerAddr = 0 ;
    int ipType = 0;//0:lan,1:wan default:lan
    sendHttpResponseHeader( session , "200 OK" , "text/html" ) ;

    if(language == 1)
    {
        //printf("Show English Control Page !\n");
    }
    else if(language == 0)
    {
        //printf("Show Chinese Control Page !\n");
    }
    else if(language == 2)
    {
        //printf("Show Czech Control Page !\n");
    }

    int ChannelNum = MAX_REC_CHANNEL;

    char szShowCtrlPage[4096] ;
    char szRequestIP[16] = {0};
    char szLastLoginUser[16] = {0} ;
	
    sprintf( szShowCtrlPage , SHOW_CONTROL_PAGE , host , port , ChannelNum , language , loginUser , (char *)get_ie_version()) ;				

    int len = strlen( szShowCtrlPage ) ;
    TcpBlockSend( session , szShowCtrlPage , strlen( szShowCtrlPage ) ) ;

}


