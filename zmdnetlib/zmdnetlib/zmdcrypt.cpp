

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include "zmdcrypt.h"

#define CRYPT_RESULT_LEN		16 
#define CRYPT_BUF_LEN		260

const char crypt_mask[16] = "adce3ddfei833" ;
const char xor_mask[16] = ",,33df389df=3df" ;

void turnString( char* src )
{
	
}

bool ZmdEnCrypt( char* src , char* key )
{
	int srclen = strlen( src ) ;
	int masklen = strlen( crypt_mask ) ;
	int keylen = strlen( key ) ;
	int xorlen = strlen( xor_mask ) ;

	//char* szMask = (char*)malloc( srclen ) ;//[CRYPT_RESULT_LEN] ;
	//memset( szMask , 0 , srclen ) ;
	
	int i = 0 ; 
    for( i = 0 ; i < srclen ; i ++ )
    {
		src[i] = src[i] + crypt_mask[i%masklen] ;
	}

    for( i = 0 ; i < srclen ; i ++ )
    {
		src[i] = src[i] ^ xor_mask[i%xorlen] ;
	}

    for( i = 0 ; i < srclen ; i ++ )
    {
		src[i] = src[i] ^ key[i%keylen] ;
	}

    //printf( "encrypt = %s\r\n" , src ) ;
	
	return true ;
}

bool ZmdDeCrypt( char* crypt , char* key )
{
	int cryptlen = strlen( crypt ) ;
	int masklen = strlen( crypt_mask ) ;
	int keylen = strlen( key ) ;
	int xorlen = strlen( xor_mask ) ;
	int i = 0 ;

    for( i = 0 ; i < cryptlen ; i ++ )
    {
		crypt[i] = crypt[i] ^ key[i%keylen ] ;
	}

    for( i = 0 ; i < cryptlen ; i ++ )
    {
		crypt[i] = crypt[i] ^ xor_mask[i%xorlen ] ;
	}

    for( i = 0 ; i < cryptlen ; i ++ )
    {
		crypt[i] = crypt[i] - crypt_mask[i%masklen] ;
	}

	//printf(" decrypt = %s\r\n" , crypt ) ;

	return true ;
}







