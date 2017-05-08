
//+---------------------------------------------------------------------------
//
//  File:   	mutex.h
//
//  Author:		jianghm tablejiang@21cn.com
//
//  Contents:   
//
//  Notes:		互斥对象，信号灯，引用计数对象，自动锁
//
//  Version:	1.11
//  			
//  Date:		2012-12-13
//
//  History:		
// 			 jianghm	2012-12-13   1.0	   创建文件
//
//----------------------------------------------------------------------------

#ifndef _JIANGHM_TCPLIB_CORE_OBJECT_HEADER_3474329203480234
#define _JIANGHM_TCPLIB_CORE_OBJECT_HEADER_3474329203480234


#include "tcplibdef.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#endif

#include <list>

#ifdef _BUILD_FOR_ARM_
#define myhtons(var) ((var & 0xff) << 8)|((var & 0xff00) >> 8)
#define myhtonl(var) ((n & 0xff) << 24) |((n & 0xff00) << 8) | \
					 ((n & 0xff0000UL) >> 8) | ((n & 0xff000000UL) >> 24)

#define myntohs		 myhtons
#define myntohl		 myhtonl

#else

#define myhtons(var) (var)
#define myntohs(var) (var)
#define myhtonl(var) (var)
#define myhtonl(var) (var)

#endif

//--------------------------------------------------------------
// 锁的C实现
typedef struct
{
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
    pthread_mutexattr_t mta ;
#endif
} Zmd_Mutex ;

//创建锁
Zmd_Mutex	*CreateZmdMutex(  );
//销毁锁
void		DestroyZmdMutex( Zmd_Mutex *m );
//进入锁
void		LockZmdMutex( Zmd_Mutex *m );
//退出锁
void		UnlockZmdMutex( Zmd_Mutex *m ) ;

//===========================================================
// 信号灯的C实现

typedef struct
{
#ifdef _WIN32
    HANDLE sem;
#else
    sem_t sem;
#endif
} Zmd_Sem ;


Zmd_Sem	*CreateZmdSem( ) ;
void		DestroyZmdSem( Zmd_Sem *sem ) ;
void		ZmdSemUp( Zmd_Sem *sem ) ;
void		ZmdSemDown( Zmd_Sem *sem ) ;

//===========================================================
//线程实现

//线程回调函数原型
typedef void *(*threadCallback)( void *userData ) ;

//创建线程
void *CreateZmdThread( void *userData , threadCallback callback ) ;

//销毁线程
//最好在线程提供的threadCallback先退出线程,否则该函数
//提供会强制终止线程,需要优雅的关闭，需要业务层自己做协调
void DestroyThread( void *threadHandle ) ;


//------------------------------------------------------
//信号灯C++实现
//
//------------------------------------------------------
class CSemaphore
{
public:
    CSemaphore() ;

    ~CSemaphore() ;

    void down() ;
    void up() ;

private:
#ifdef _WIN32
    HANDLE sem;
#else
    sem_t sem;
#endif

};

//---------------------------------------------------------
//互斥量C++实现

class CMutex
{
public:
    CMutex() ;
    ~CMutex() ;

    void lock() ;
    void unlock() ;

private:

#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
    pthread_mutexattr_t mta ;
#endif
};

//-------------------------------------
// 自动锁，对象建立加锁，对象销毁解锁
class CAutoMutex
{
public:
    CAutoMutex( CMutex *mutex )
    {
        m_pmutex = mutex ;
        if( m_pmutex )
            m_pmutex->lock() ;
    }

    virtual ~CAutoMutex()
    {
        if( m_pmutex )
        {
            m_pmutex->unlock() ;
            m_pmutex = NULL ;
        }
    };
private:
    CMutex				*m_pmutex ;
};

class CRefObject
{
public:
    CRefObject()
    {
        m_nRefCount = 1 ;
    };
    ~CRefObject() {}	;

    int addRef()
    {
        m_csRef.lock() ;
        //++m_nRefCount ;
        register int ret = ++m_nRefCount ;
#ifdef _DEBUG

#endif
        m_csRef.unlock() ;
        return ret ;
    } ;

    int release()
    {
        m_csRef.lock() ;

        register int ret = --m_nRefCount;

#ifdef _DEBUG

#endif

        if (!ret)
        {
            m_csRef.unlock() ;
            destroyThis();
            return ret ;
        }
        m_csRef.unlock() ;
        return ret ;
    };

protected:
    //public:

    virtual void destroyThis() = 0 ;

    CMutex	m_csRef ;
    int	m_nRefCount ;
};

#endif







