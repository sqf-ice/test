#ifndef __DES_H
#define __DES_H

#define uchar unsigned char
#define uint  unsigned int

void desfun(uchar *subkey);
void IniPer(uchar *src);
void FinalPer(uchar *result);
void SubKey_Generation(uchar *key);
void des_16(uchar *src, uchar *subkey, uchar *result);
void Single_DES_Encrypt(uchar *src, uchar *key, uchar *result);
void Single_DES_Decrypt(uchar *src, uchar *key, uchar *result);
void  Single_3DES_Decrypt(uchar * src,uchar *key, uchar *result);
void  Single_3DES_Encrypt(uchar * src,uchar *key, uchar *result);


void HexXor(uchar * Data1,uchar *Data2, uchar *DataOut);
void PBOC_MAC(uchar * Key, uchar *InitialValue, uchar *DataIn,uchar Len, uchar *DataOut);

#endif
