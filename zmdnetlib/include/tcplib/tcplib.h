

#ifndef _JIANGHM_TCPLIB_EXPORT_HEADER_2343987234234
#define _JIANGHM_TCPLIB_EXPORT_HEADER_2343987234234

#include "tcplibdef.h"


//网络事件通知回调函数原型
//


/*
* Function:		有tcp连接到达时，通知业务层的回调原型
* Called By:
* Input:

	@session :		Tcp会话句柄
	@client_addr:	客户端连接地址

* Output:

	@userData :	用户数据.该用户数据由业务层产生，提交给网络层，后面网络层的
				回调会将用户数据返回给业务层，业务层可使用该数据来确定是哪个网络
				连接的数据.例如使用用户id

* Return:		业务层根据自身业务要求返回是否接受该连接
				true 接受该连接
				false 拒绝该连接
				例如: 已超出预定最大连接时返回false。
* Others:
*/
typedef bool (*onAcceptTcpLibCallback)( int &userData , void *session , sockaddr_in *client_addr ) ;


/*
* Function:		通知连接服务器结果

* Called By:
* Input:

	@userData :			用户数据
	@bConnected :		连接结果(always 1),只有连接成功才会调用该事件

* Output:
* Return:
* Others:
*/
typedef void (*onConnectTcpLibCallback)( int userData , int bConnected ) ;

/*
* Function:		网络层通知业务层，连接已关闭
* Called By:
* Input:

	@userData :		用户数据

* Output:
* Return:
* Others:
*/
typedef void (*onCloseTcpLibCallback)( int userData ) ;

/*
* Function:		网络层通知业务层，有Tcp连接数据到达
* Called By:
* Input:

	@userData :	用户数据
	@data :		底层接收到的数据内容
	@len :		底层接收到的数据长度
* Output:
	@used :		返回数据已经处理了多少.(由于Tcp是非消息边界保护的，所以有可能会收到切断的数据包.)
				造成一次接收的数据内有截断的数据包，该参数就是让业务层告诉网络层，已经处理了多少数据.
				网络层会将已处理的数据丢弃。
* Return:
* Others:
*/
typedef bool (*onReceiveTcpLibCallback)( int userData , char *data , int len , int &used ) ;

/*
* Function:		实际作用是OnSend,网络层每处理一次recv后，会调用一次该回调
				业务层可以在该回调中处理一些定时或者轮询业务。
* Called By:
* Input:

	@userData :	用户数据

* Output:
* Return:


* Others:


*/
typedef bool (*onIdleTcpLibCallback)( int userData ) ;


/*
* Function:		三个接口都是用来创建一个tcp客户端，用来与服务器通讯
				提供服务器地址的3种输入方式
* Called By:
* Input:

	@userData :		用户数据,回调时，会将这个数据返回给业务层
	@connect_cb :	onConnect回调
	@close_cb :		onClose回调
	@receive_cb:	onReceive回调
	@idle_cb:		onIdle回调

* Output:
* Return:

* Others:

*/
void *CreateClient(struct sockaddr_in *addr , int userData , int isReconnect ,
                   onConnectTcpLibCallback connect_cb , onCloseTcpLibCallback close_cb ,
                   onReceiveTcpLibCallback receive_cb , onIdleTcpLibCallback idle_cb ) ;



/*
* Function:		停止一个客户端,该函数提供温和的关闭，设置停止标志，等待
				线程自行终结。
注意:			调用该接口，onCloseClientCallback将不会响应，
				上层需要在调用该接口时，做退出处理

* Called By:
* Input:

	@clientHandle :	客户端句柄

* Output:
* Return:

* Others:

*/
void StopClient( void *clientHandle ) ;

/*
* Function:		停止一个客户端.粗暴的关闭，直接终止一个线程
注意:				该接口是提供给外部调用，用来主动销毁一个会话
					上层调用该借口，需要自行做资源的释放，因为
					不会再响应onCloseCallback()来通知用户关闭了。
					在会话线程本身调用，也将会立即终止线程,所以部分释放资源
					函数可能不会调用，有可能会引起资源的无法释放。
					请在外部线程调用

* Called By:
* Input:

	@clientHandle :	客户端句柄

* Output:
* Return:
* Others:
*/
void TerminateClient( void *clientHandle ) ;

/*
* Function:		阻塞发送一块数据
注意 :			发送接口只能开放clientHandle，
				后期修改也不要开放socket句柄给上层,只允许上层使用该函数进行数据发送
				否则上层使用fd直接发送进不了发送锁，会造成tcp数据包内容的交叉

* Called By:
* Input:

	@sessionHandle :客户端句柄
	@data :			数据指针
	@datalen :		数据长度

* Output:
* Return:
* Others:
*/
int ClientBlockSend( void *clientHandle , char *data , int datalen );

//bool ClientBlockRecv( void* clientHandle , char* buf , int* len );

/*
* Function:		重置客户端连接.用于给上层对客户端进行重置.
				底层会断开当前客户端连接，并修正地址后，重新
				建立连接

* Called By:
* Input:

	@clientHandle : 客户端句柄
	@addr :			是否需要重置目标地址(默认=NULL，不重置)

* Output:
* Return:
* Others:
*/
void ResetClient( void *clientHandle , struct sockaddr_in *addr = 0 ) ;



//==============================================================
//网络方法调用
/*
* Function:		创建一个tcp服务器

* Called By:
* Input:

	@acceptcb :		连接到达回调
	@onclosecb :	连接关闭回调
	@onrecvcb :		连接接收到数据回调
	@idlecb :		连接空闲回调

* Output:
* Return:	服务器句柄。返回NULL为创建失败

* Others:

*/
void *CreateTcpServer( onAcceptTcpLibCallback acceptcb , onCloseTcpLibCallback onclosecb ,
                       onReceiveTcpLibCallback onrecvcb , onIdleTcpLibCallback idlecb ) ;

/*
* Function:		启动Tcp服务器

* Called By:
* Input:

	@serverHandle :	服务器句柄
	@addr :			服务器监听地址(0.0.0.0 为监听所有本机地址)
	@port :			监听端口
	@isBlock :		是否重复绑定该地址，直到成功(默认true)

* Output:
* Return:	创建结果
			true: 创建成功
			false: 创建失败
* Others:
*/
bool StartTcpServer( void *serverHandle , char *addr , unsigned short port , bool isBlock = true ) ;


/*
* Function:		停止一个会话.该接口温和关闭会话，将至会话的run参数为false
				等待线程温和终止

* Called By:
* Input:

	@sessionHandle :	会话句柄

* Output:
* Return:
* Others:
*/
void StopTcpSession( void *sessionHandle ) ;

/*
* Function:		停止一个会话.粗暴的关闭，直接终止一个线程
注意:				该接口是提供给外部调用，用来主动销毁一个会话
					上层调用该借口，需要自行做资源的释放，因为
					不会再响应onCloseCallback()来通知用户关闭了。
					在会话线程本身调用，也将会立即终止线程,所以部分释放资源
					函数可能不会调用，有可能会引起资源的无法释放。
					请在外部线程调用

* Called By:
* Input:

	@sessionHandle :	会话句柄

* Output:
* Return:
* Others:
*/

void TerminateTcpSession( void *sessionHandle ) ;


/*
* Function:		阻塞发送一块数据
注意 :			发送接口只能开放sessionHandle，
				后期修改也不要开放socket句柄给上层,只允许上层使用该函数进行数据发送
				否则上层使用fd直接发送进不了发送锁，会造成tcp数据包内容的交叉

* Called By:
* Input:

	@sessionHandle :会话句柄
	@data :			数据指针
	@datalen :		数据长度

* Output:
* Return:
* Others:
*/
int TcpBlockSend( void *sessionHandle , char *data , int datalen , int timeout = 5 );					

int TcpBlockRecv( void *sessionHandle , char *buf , int buflen , int timeout ) ;

int GetSessionSock(void *sessionHandle);


#ifdef DM368
/*
* Function:		设置TCP的发送缓冲区大小				
* Called By:	
* Input:		

	@fd :SOCK 
	@len :		需要设置的发送缓冲区大小

* Output:		
* Return:				
* Others:		
*/
static void TcpSetSendBuff( int fd, int len );

#endif
#endif

