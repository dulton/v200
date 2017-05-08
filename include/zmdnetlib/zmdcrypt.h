

#ifndef _JIANGHM_ZMD_CRYPT_HEADER_3298743987432
#define _JIANGHM_ZMD_CRYPT_HEADER_3298743987432

//+---------------------------------------------------------------------------
//
//  File:   	zmdcrypt.h
//
//  Author:		TableJiang
//
//  Contents:   
//
//  Notes:		zmd加密解密
//
//  Version:	1.00
//  			
//  Date:		2013-2-1
//
//  History:		
// 			 Tablejiang 2013-2-1		创建文件
//
//---------------------------------------------------------------------------


#define CRYPT_KEY		"zmodo19820816"

#ifdef __cplusplus
extern "C" {
#endif


//========================================================
// 加密编码函数
// src :    需要加密编码的字串
// key :    秘钥字串
// detail:  加密编码函数，加密结果一定是16字节的内存.有可能会有\0
// 所以不要用strcpy拷贝，使用memcpy。
bool ZmdEnCrypt( char* src , char* key ) ;

//========================================================
// 解密函数
// crypt : 加密串，必定是16字节的
// key :	秘钥字串，
bool ZmdDeCrypt( char* crypt , char* key ) ;



#ifdef __cplusplus
}
#endif

#endif


