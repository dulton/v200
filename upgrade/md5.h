
#ifndef __MD5_INCLUDED__
#define __MD5_INCLUDED__
#include <stdio.h>

//MD5摘要值结构体
typedef struct MD5VAL_STRUCT
{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
} MD5VAL;

//计算字符串的MD5值(若不指定长度则由函数计算)
MD5VAL md5(char *str, unsigned int size );

//MD5文件摘要
MD5VAL md5File(FILE *fpin);

#endif

