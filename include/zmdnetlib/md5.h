#ifndef MD5_H_
#define MD5_H_
/*输入任意一个字符串，经过md5算法处理后，返回结果：一个定长（32个字符）
字符串 */
char *MDString (char *);

/*输入任意一个文件名，文件内容经过md5算法处理后，返回结果：一个定长
（32个字符）字符串 */
char *MDFile (char *);

/*输入任意一个字符串text,和一个用做密钥的字符串key,经过hmac_md5算法处
理，返回处理结果：一个定长字符串（32个字符）*/
char *hmac_md5(char *text, char *key);

#endif
