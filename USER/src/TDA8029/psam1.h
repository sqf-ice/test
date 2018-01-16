 
#ifndef __PSAM1_H
#define __PSAM1_H

#include "stm32f10x.h"  



//通讯缓存最大长度
#define 	MAXRLEN 				64

#define     SAM_OK				0x00
#define     SAM_NG				0x01
#define     SAM_MAX_BUFF		64
#define     HALF        0    
#define     FULL        1 

				
#define		EP           0x02  //电子钱包

#define 	Type_MF			0x00
#define 	Type_DF			0x01
#define 	Type_EF			0x02

#define 	Start			0x00
#define 	End			    0x01

#define 	LoadKey		    0x01
//#define 	Modify			0x01
 
unsigned char PSAMInit(unsigned char* pDataOut);
unsigned char  PSAM_CreateFile(unsigned char *pDataIn, unsigned char Lc, unsigned char p1, unsigned char p2, unsigned char* pDataOut );
unsigned char PSAM_Select_File(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char* pDataOut, unsigned int *Le );
unsigned char Init_for_descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char PKey,unsigned char VKey,unsigned char* pDataOut );
unsigned char PSAM_Descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char type, unsigned char* pDataOut, unsigned char Le );
 unsigned char PSAM_ReadBinary( unsigned char file,unsigned char* pDataOut, unsigned int* Le ); 
unsigned char PSAM_Get_challenge(unsigned char Le,unsigned char* pDataOut);
unsigned char Sam_Get_Response(  unsigned char Lc, unsigned char* pDataOut,unsigned int *Le )  ;
unsigned  int  PSAM_Format(void);
unsigned int  PSAM_Format_1(unsigned char *pDataIn, unsigned char Lc, unsigned char* pDataOut);
unsigned int  PSAM_Format_2(unsigned char *pDataIn, unsigned char Lc, unsigned char* pDataOut);
char PSAM_WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len);
unsigned int PSAM_External_Authenticate(unsigned char Key_Flag,unsigned char *pDataIn,unsigned char *pDataOut);
unsigned int PSAM_Verify_Pin(unsigned char File_Flag,unsigned char Lc, unsigned char *pDataIn);
unsigned char Init_SAM_For_Purchase(unsigned char *pDataIn, unsigned char Lc,  unsigned char* pDataOut);
unsigned char Credit_SAM_For_Purchase(unsigned char *pDataIn, unsigned char* pDataOut);
void PSAM_Config(void); 
#endif
