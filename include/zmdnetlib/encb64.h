#ifndef __ENCB64_H__
#define __ENCB64_H__

#include <sys/types.h>

/*RFC 2045
 * 0---25	A---Z
 * 26---51	a---z
 * 52---61	0---9
 * 62		+
 * 63		/
 * (pad)	=
 */

/* This function encrypt src to dest by Base64 */
//int encrypt_b64(char* dest,char* src,size_t size);

int encrypt_b64_file_to_buf (char* en64_buf, const char* s_file,int len);
int encrypt_b64(char* dest,char* src,int size);

#endif
