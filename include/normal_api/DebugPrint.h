#ifndef _DEBUG_PRINT_H_
#define _DEBUG_PRINT_H_

#define KEY_INFO_PRINT
#define DEBUG_ERROR_PRINT
#define DEBUG_INFO_PRINT
#define ASSERT_PRINT

#ifdef KEY_INFO_PRINT
#define	KEY_INFO(x...)   fprintf(stderr,x)
#else
#define	KEY_INFO(x...)
#endif

#ifdef DEBUG_ERROR_PRINT
#define	DEBUG_ERROR(x...)   fprintf(stderr,x)
#else
#define	DEBUG_ERROR(x...)
#endif

#ifdef DEBUG_INFO_PRINT
#define	DEBUG_INFO(x...)   fprintf(stderr,x)
#else
#define	DEBUG_INFO(x...)
#endif

#define DEPRINTF(x...) fprintf(stderr,x)
#define DEBUGMSG(cond,printf_exp)	((void)((cond)?(DEPRINTF printf_exp),1:0))

#ifndef ASSERT
#ifdef ASSERT_PRINT
#define ASSERT(exp)				((void)((exp)?1:(printf ("ASSERT failed: line %d, file %s\n", __LINE__,__FILE__), 0)))	//assert(exp)//((void)((exp)?1:(printf ("[ENCODE]ASSERT failed: line %d, file %s\n", __LINE__,__FILE__), /*debug_break(),*/ 0)))
#else
#define ASSERT(exp)
#endif
#endif 


#endif
