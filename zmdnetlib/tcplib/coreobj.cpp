
#include "coreobj.h"

//-----------------------
//线程对象描述结构
//-----------------------
typedef struct
{
#ifdef _WIN32
    HANDLE			thread_handle ;		//Win32线程句柄
#else
    pthread_t		thread_handle ;		//linux线程句柄
#endif
    threadCallback	callback ;			//回调接口
    void			*userData ;			//用户数据
} ZMD_THREAD_INFO ;

//-------------------------------------------
//指定linux系统下线程堆栈大小
//0为使用默认线程堆栈
static int g_threadStackSize = 0 ;

//如果定义了内存调试宏
#ifdef DEBUG_MALLOC
CMutex	mallocMutex ;
int		malloctick = 0 ;

//-----------------------------------------------
//对内存分配增加一个计数器,用于调试内存泄露
void *debugMalloc( int size )
{
    void *pmem = 0 ;
    mallocMutex.lock() ;

    //malloc次数+1
    malloctick ++ ;
    pmem = malloc( size ) ;

    //printf( "debugMalloc count = %d\r\n" , malloctick ) ;

    mallocMutex.unlock() ;

    return pmem ;
}

//-----------------------------------------------
//用于调试内存泄露
void	debugFree( void *pmem )
{
    mallocMutex.lock() ;
    //malloc次数-1
    malloctick -- ;
    free( pmem ) ;
    //printf( "debugFree count = %d\r\n" , malloctick ) ;
    mallocMutex.unlock() ;
}
#endif	//DEBUG_MALLOC

#ifdef _WIN32
//-----------------------------------
// WIN32线程回调函数
//-----------------------------------
DWORD WINAPI threadFunction(void *pV)
{
    ZMD_THREAD_INFO *pThread = (ZMD_THREAD_INFO *)pV ;
    if( !pThread )
    {
        return 0 ;
    }

    //调用用户的线程回调函数
    if( pThread->callback )
    {
        pThread->callback( pThread->userData ) ;
    }

    //保存线程句柄，释放完对象后，关闭线程句柄
    HANDLE hThread = pThread->thread_handle ;
    DebugFree( pThread ) ;
    //printf( "******************thread exit****************\r\n" ) ;
    if( hThread )
        CloseHandle( hThread ) ;
    return 0 ;
}
#else
//-------------------------------------------
// linux线程回调函数
//-------------------------------------------
void *threadFunction(void *pV)
{
    pthread_detach(pthread_self()) ;

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &sa, 0 );

    //允许线程可以被取消
    pthread_setcancelstate( PTHREAD_CANCEL_ENABLE , NULL ) ;
    //取消模式设置为立即终止
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS , NULL ) ;

    ZMD_THREAD_INFO *pThread = (ZMD_THREAD_INFO *)pV ;
    if( !pThread )
    {
        pthread_exit(0) ;
        return 0;
    }

    //调用用户的线程回调函数
    if( pThread->callback )
    {
        pThread->callback( pThread->userData ) ;
    }

    //释放线程句柄
    DebugFree( pThread ) ;
    pthread_exit(0) ;
    return 0 ;
}
#endif	//WIN32

//----------------------------------------------------
// 创建线程
// @userData : 用户数据传入(回调将会将此数据返回给用户)
// @cb		: 用户回调函数(原型 threadCallback)
//----------------------------------------------------
void *CreateZmdThread( void *userData , threadCallback callback )
{
    ZMD_THREAD_INFO *pThread = (ZMD_THREAD_INFO *)DebugMalloc( sizeof( ZMD_THREAD_INFO ) ) ;
    if( !pThread )
    {
        printf("function %s ,line:%d\n", __FUNCTION__, __LINE__);
        return 0 ;
    }
    memset( pThread , 0 , sizeof( ZMD_THREAD_INFO ) ) ;

    pThread->callback = callback ;
    pThread->userData = userData ;

#ifdef _WIN32
    //win32启动线程
    unsigned long threadID = 0 ;
    //创建线程
    pThread->thread_handle = (HANDLE)CreateThread( NULL , 0 , threadFunction, (void *)pThread, 0, &threadID);
    if( !threadID )
    {
        DebugFree( pThread ) ;
        return 0 ;
    }
#else
    /*
    //linux启动线程
    //linux线程启动可以指定线程堆栈
    //线程堆栈大小由g_threadStackSize指定
    bool bSetStackSuccessed = false ;

    //更改线程默认堆栈大小
    pthread_attr_t attr ;
    pthread_attr_init( &attr ) ;

    if( g_threadStackSize ){

    	if( pthread_attr_setstacksize( &attr , g_threadStackSize )){
    		printf( "pthread_attr_setstacksize failed!!\r\n" ) ;
    		pthread_attr_destroy( &attr ) ;
    		bSetStackSuccessed = false ;
    	}
    	else{
    		bSetStackSuccessed = true ;
    	}
    }

    int ret = 0 ;
    //根据堆栈设条件来创建线程
    if( bSetStackSuccessed ){
    	//指定线程堆栈大小
    	ret = pthread_create( &pThread->thread_handle, &attr, threadFunction, (void*)pThread);
    }
    else{
    	printf( "pthread_create() !!\r\n" ) ;
    	//使用系统默认线程堆栈大小
    	ret = pthread_create( &pThread->thread_handle, NULL, threadFunction, (void*)pThread);
    	printf( "********pthread_create() complete!!\r\n" ) ;
    }

    pthread_attr_destroy( &attr ) ;

    if( ret != 0 ){
    	//创建失败
    	printf( "pthread_create failed! ret =%d ! stacksize = %d \r\n" , ret , g_threadStackSize ) ;
    	DebugFree( pThread ) ;
    	return 0 ;
    }
    */

    int ret = 0 ;

    //printf( "pthread_create() !!\r\n" ) ;
    //使用系统默认线程堆栈大小
    ret = pthread_create( &pThread->thread_handle, NULL, threadFunction, (void *)pThread);
    //printf( "********pthread_create() complete!!\r\n" ) ;

    if( ret != 0 )
    {
        //创建失败
        printf( "pthread_create failed! ret =%d ! stacksize = %d \r\n" , ret , g_threadStackSize ) ;
        DebugFree( pThread ) ;
        return 0 ;
    }

#endif

    return pThread ;
}

//----------------------------------------------
// 外部立即终止线程
// @threadHandle :	线程对象句柄
//----------------------------------------------
void DestroyThread( void *threadHandle )
{
    ZMD_THREAD_INFO *pThread = (ZMD_THREAD_INFO *)threadHandle ;
    if( !pThread )
    {
        return ;
    }

    printf( "Destroy Thread !!\r\n" ) ;

    if( pThread->thread_handle )
    {
#ifdef _WIN32
        HANDLE hThread = pThread->thread_handle ;
        TerminateThread( hThread , 0 ) ;
        CloseHandle(hThread);
        DebugFree( pThread ) ;
#else
        pthread_t hThread = pThread->thread_handle ;
        pthread_cancel(  hThread  ) ;
        pthread_join( hThread , NULL ) ;
        DebugFree( pThread ) ;
#endif
    }
}


#ifdef _WIN32
//-----------------------------------
// win32下内核对象封装 C实现
//-----------------------------------

Zmd_Mutex *CreateZmdMutex(  )
{
    Zmd_Mutex *pMutex = (Zmd_Mutex *)malloc( sizeof( Zmd_Mutex ) ) ;
    if( !pMutex )
        return 0 ;

#ifdef _WIN32
    InitializeCriticalSection(&pMutex->cs);
#else
    pthread_mutexattr_init( &pMutex->mta ) ;
    pthread_mutexattr_settype( &pMutex->mta , PTHREAD_MUTEX_RECURSIVE_NP ) ;
    pthread_mutex_init( &pMutex->cs , &mta ) ;
#endif

    return pMutex ;
}

void DestroyZmdMutex( Zmd_Mutex *m )
{
    if( !m )
        return ;
#ifdef _WIN32
    DeleteCriticalSection(&m->cs);
#else
    pthread_mutexattr_destroy( &m->mta ) ;
    pthread_mutex_destroy(&m->cs);
#endif

    free( m ) ;

}

void LockZmdMutex( Zmd_Mutex *m )
{
    if( !m )
        return ;

#ifdef _WIN32
    EnterCriticalSection(&m->cs);
#else
    pthread_mutex_lock(&m->cs);
#endif

}

void UnlockZmdMutex( Zmd_Mutex *m )
{
    if( !m )
        return ;

#ifdef _WIN32
    LeaveCriticalSection(&m->cs);
#else
    pthread_mutex_unlock(&m->cs);
#endif

}


Zmd_Sem *CreateZmdSem( )
{
    Zmd_Sem *pSem = (Zmd_Sem *)malloc( sizeof(Zmd_Sem) ) ;
    if( !pSem )
        return 0 ;

#ifdef _WIN32
    pSem->sem = CreateSemaphore(NULL, 0, 65535 , NULL);
#else
    sem_init(&pSem->sem, 0, 0);
#endif
    return pSem ;
}

void DestroyZmdSem( Zmd_Sem *sem )
{
    if( !sem )
        return ;

#ifdef _WIN32
    CloseHandle(sem->sem);
#else
    sem_destroy(&sem->sem);
#endif
    free( sem ) ;

}

void ZmdSemUp( Zmd_Sem *sem )
{
    if( !sem )
        return ;

#ifdef _WIN32
    ReleaseSemaphore(sem->sem, 1, NULL);
#else
    sem_post(&sem->sem);
#endif

}

void ZmdSemDown( Zmd_Sem *sem )
{
    if( !sem )
        return ;

#ifdef _WIN32
    WaitForSingleObject(sem->sem, INFINITE);
#else
    int ret = 0 ;
    while (0 != (ret = sem_wait(&sem->sem)))
    {
        if (errno != EINTR)
        {
            printf ("Sem_wait returned %ld\n", (unsigned long)ret);
            printf("sem_wait for handler failed");
        }
    }
#endif

}

//-----------------------------------
// win32下内核对象封装 C++实现
//-----------------------------------
CSemaphore::CSemaphore( )
{
    sem = CreateSemaphore(NULL, 0, 65535 , NULL);
}

CSemaphore::~CSemaphore()
{
    if (sem)
        CloseHandle(sem);
}

void CSemaphore::down()
{
    WaitForSingleObject(sem, INFINITE);
}

void CSemaphore::up()
{
    ReleaseSemaphore(sem, 1, NULL);
}


CMutex::CMutex()
{
    InitializeCriticalSection(&cs);
}

CMutex::~CMutex()
{
    DeleteCriticalSection(&cs);
}

void CMutex::lock()
{
    EnterCriticalSection(&cs);
}

void CMutex::unlock()
{
    LeaveCriticalSection(&cs);
}

#else		// Linux implementation

//-----------------------------------
// linux下内核对象封装
//-----------------------------------
CSemaphore::CSemaphore()
{
    sem_init(&sem, 0, 0);
}

CSemaphore::~CSemaphore()
{
    sem_destroy(&sem);
}

void CSemaphore::down()
{
    int ret = 0 ;
    while (0 != (ret = sem_wait(&sem)))
    {
        if (errno != EINTR)
        {
            printf ("Sem_wait returned %ld\n", (unsigned long)ret);
            printf("sem_wait for handler failed");
            exit(1);
        }
    }

    //sem_wait(&sem);
}

void CSemaphore::up()
{
    sem_post(&sem);
}


CMutex::CMutex()
{
    pthread_mutexattr_init( &mta ) ;
    pthread_mutexattr_settype( &mta , PTHREAD_MUTEX_RECURSIVE_NP ) ;
    pthread_mutex_init( &cs , &mta ) ;
}

CMutex::~CMutex()
{
    pthread_mutexattr_destroy( &mta ) ;
    pthread_mutex_destroy(&cs);
}

void CMutex::lock()
{
    //printf( " lock the %d \r\n" , &cs ) ;
    pthread_mutex_lock(&cs);
}

void CMutex::unlock()
{
    //printf( " unlock the %d \r\n" , &cs ) ;
    pthread_mutex_unlock(&cs);
}

#endif

