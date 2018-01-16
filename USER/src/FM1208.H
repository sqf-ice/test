struct  TranSciveBuffer
{
	unsigned char MfCommand;
  	unsigned int  MfLength;
  	unsigned char MfData[64];
}; 

char pro_reset(unsigned char *DataOut, unsigned char *rlen);
char  Pcd_Cmd(unsigned char* pDataIn, unsigned char* pDataOut,unsigned char * Out_Len);
char  WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len);


unsigned char  Set_CpuCard(void);
unsigned int  FM1208_CreateFile(unsigned char *fileid, unsigned char Len, unsigned char *pDataIn );
unsigned int FM1208_Load_Key(unsigned char Keysign, unsigned char Len, unsigned char* pDataIn);
unsigned int AppendRecord(unsigned char fileid, unsigned char Len, unsigned char* pDataIn);
unsigned int FM1208_SelectFile(unsigned char* pDataIn, unsigned char* pDataOut,unsigned char * Out_Len);
unsigned int FM1208_GetChallenge(unsigned char Len,unsigned char* pRan);
unsigned int FM1208_Update_Binary(unsigned char File_Flag,unsigned char *pDataIn, unsigned char Len);
void MAC1(unsigned char File_Flag, unsigned char Len, unsigned char *pDataIn, unsigned char *LineCKey,unsigned char *Challenge,unsigned char *pDataOut,unsigned char *Le);
unsigned int FM1208_External_Authenticate(unsigned char Key_Flag,unsigned char *pDataIn,unsigned char *pDataOut);
unsigned  int  FM1208_Format(void);
unsigned int FM1208_ReadNomey(unsigned char type ,unsigned char *pDataOut);
unsigned int FM1208_Inital_Authenticate(unsigned char Key_Flag, unsigned char Lc, unsigned char *pDataIn,unsigned char *pDataOut);
unsigned int FM1208_Verify_Pin(unsigned char File_Flag,unsigned char Len, unsigned char *pDataIn);
unsigned int FM1208_CreditForLoad(unsigned char *pDataIn,unsigned char *pDataOut);
unsigned int FM1208_InitalizeForLoad(unsigned char type, unsigned char *pDataIn,unsigned char *pDataOut);
unsigned int FM1208_Read_Binary(unsigned char File_Flag,unsigned char *pDataOut, unsigned char Len );
unsigned int FM1208_Initalize_For_Purchase(unsigned char type, unsigned char *pDataIn,unsigned char *pDataOut, unsigned char *Len);
unsigned int FM1208_Debit_For_Capp_Purchase (unsigned char *pDataIn,unsigned char *pDataOut,unsigned char *Len);
#define		ST_ERR		 0xFF	//返回错误
#define		ED           0x01   //电子存折				
#define		EP           0x02   //电子钱包


