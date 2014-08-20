#ifndef COMMON_MD5CALC_H
#define COMMON_MD5CALC_H

void MD5_String(const char * string, char * output);
void MD5_Binary(const char * string, unsigned char * output);
void MD5_Salt(unsigned int len, char * output);

#endif /* COMMON_MD5CALC_H */
