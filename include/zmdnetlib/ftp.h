#ifndef FTP_H_
#define FTP_H
#include <stdio.h>
#include <stdlib.h>
int ftp_upload(char *username, char *passwd, char *serverip, char *targetpath, char *sourcepath);
#endif

