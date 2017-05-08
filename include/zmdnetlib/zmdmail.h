#ifndef _ZMD_MAIL_H_
#define _ZMD_MAIL_H_

/**
 * @file zmdmail.h
 * @brief declare zmd mail api
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interface function that can send mail with smtp
 *
 * @param from	mail sender,format is "username@hostname"
 * @param to		mail reciver,format is "username@hostname"
 * @param filepath		image path
 * @param passwd		sender user password
 * @param subject		mail subject
 * @param sslflag		need send mail with ssl?0:no,1:yes
 *
 * @return 0:send mail success, -1:failed to send mail
 */
	int zmd_send_mail( char *from, char *to, char* hostname, int port, char *filepath[], int filenum, 
			char *passwd, char *subject, char* content,int sslflag );


#ifdef __cplusplus
}
#endif

#endif /* __ZMD_MAIL_H__ */

