#include "stm32f10x.h"
#include "psam1.h"
#include "t8029.h"
#include <stdio.h>
#include <string.h>
#include "atr.h"
#include "des.h"
#include "bsp_serial.h"
 
static void psam_delay_10ms(uint16_t n10ms)
{
	uint32_t Delaytick;

	Delaytick = BspGetSysTicks();
	while(!BspGetDlyTicks(Delaytick,n10ms));

}	
void PSAM_Config(void)
{
	GPIO_SetBits(TDA1_SDWN_Port,TDA1_SDWN_Pin);
	GPIO_ResetBits(TDA1_RST_Port,TDA1_RST_Pin);
	psam_delay_10ms(10);
	GPIO_SetBits(TDA1_RST_Port,TDA1_RST_Pin);
	psam_delay_10ms(25);
	GPIO_ResetBits(TDA1_RST_Port,TDA1_RST_Pin);
	psam_delay_10ms(10);
	
}
 

ATR_TypeDef  tAtr;
int8_t protocol = -1;
unsigned char PSAMInit(unsigned char* pDataOut)   //PSAM卡初始化  例如正确返回应答为:3B 6C 00 02 13 02 86 38 18 43 56 07 1E 10 2B 23     
{ 
	unsigned char rBuf[260];
	unsigned char res =0;
	unsigned char pUpData[] = {0x00};
	unsigned int len,i;
	len = sizeof(rBuf);
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_POWERUP3V, pUpData, sizeof(pUpData), rBuf, &len); 
	if (res == TDA8029_OK)
	{
		ATR_Decode(&tAtr,rBuf,len);
		ATR_GetDefaultProtocol(&tAtr,&protocol);
		printf("\nATR:\n");
		for (i = 0; i < len; i++)
			printf("%02X ",rBuf[i]);
		printf("\n--------------------\n");
		return  0;
	} 
 
    return 1;        //复位失败   	  
}

//*************************************************************************
 
// 函数名	：Sam_Selsect_File
// 描述		：选择打开当前文件
// 入口		：pDataIn ：要发送给SAM卡的文件标识   Lc: 文件标识长度  
//			  Le : 要返回的数据长度	 type: 类型00 按文件标识符选择MF或DF, 当类型为02 则选择EF 
//			  当类型为04 则按文件名来选择
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char Sam_Get_Response(  unsigned char Lc, unsigned char* pDataOut,unsigned int *Le )  
{

	unsigned char st,res;
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
		unsigned int len;
	memset(sendApduData, 0x00, MAXRLEN);  //清0

    sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xc0;				// INS 
    sendApduData[2] = 0x00;				// P1
    sendApduData[3] = 0x00;			    // P2
    sendApduData[4] = Lc;			    // lc 
//	memcpy(&sendApduData[5], pDataIn, Lc);
	len = sizeof(rxApduData);
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5,pDataOut, &len);
	if (res == TDA8029_OK)
	{
		 
		*Le = len;
		__nop();
		return 0;		
	}
	return 	st;
//	if(SendCmd(sam_snd_buff,  Lc, pDataOut, Le )!=0x9000)	 //发送命令并判断结果
//	st=SAM_NG;	   

//	else
//	st=SAM_OK;	   

//	return 	st;

}
//*************************************************************************
 
// 函数名	：PSAM_Select_File
// 描述		：选择打开当前文件
// 入口		：pDataIn ：要发送给SAM卡的文件标识   Lc: 文件标识长度  
//			  Le : 要返回的数据长度	 type: 类型00 按文件标识符选择MF或DF, 当类型为02 则选择EF 
//			  当类型为04 则按文件名来选择
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char PSAM_Select_File(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char* pDataOut,unsigned int *Le )  
{

	unsigned char st,res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0


    sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xA4;				// INS 
    sendApduData[2] = type;				// P1
    sendApduData[3] = 0x00;			    // P2
    sendApduData[4] = Lc;			    // lc 
	memcpy(&sendApduData[5], pDataIn, Lc);
	
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len);
		if((sw[0] == 0x90)||(sw[0] == 0x61))
		{
			*Le = len;
			__nop();
			return 0;
		}
		else
			return 1;		
	}
	return 	st;
} 
// 函数名	：Init_SAM_For_Purchase
// 描述		：MAC1计算初始化，用于消费
// 入口		：pDataIn ：用户卡的伪随机数、交易序号、金额等数据   Lc: 数据长度，最少为14H+18H，具体参看SAM卡的COS手册  
 		  
//
// 出口		：pDataOut ：PSAM终端交易序号、MAC1码 
// 返回		：结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char Init_SAM_For_Purchase(unsigned char *pDataIn, unsigned char Lc,  unsigned char* pDataOut)
{
			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
 

	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x70;				// INS  
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // lC 

	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 
	
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len-2);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			if(sw[0] == 0x61)
				if(Sam_Get_Response( sw[1],pDataOut ,&len )   ==0) 
					return 0;
			__nop();
			return 0;
		}		 
		__nop();
		return 1;	
	}
	return 	2; 
	
 
}
// 函数名	：Credit_SAM_For_Purchase
// 描述		：由用户卡提供的MAC2发给SAM卡进行校验，用于消费
// 入口		：pDataIn ：MAC2数据   Lc: 数据长度规定4个字节 
//			  
//			
//
// 出口		：pDataOut ：由SAM卡返回的加密后的数据及执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char Credit_SAM_For_Purchase(unsigned char *pDataIn, unsigned char* pDataOut)
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
 

	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x72;				// INS  
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = 0x04;			    // lC 

	memcpy(&sendApduData[5], pDataIn, 0x04); //把文件标识拷到发送缓冲区中	 
	
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+0x04,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len-2);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			 
			__nop();
			return 0;
		}		 
		__nop();
		return 1;	
	}
	return 	2; 
   
}
  //*************************************************************************
 
// 函数名	：Init_for_descrypt
// 描述		：DES计算初始化
// 入口		：pDataIn ：要发送给SAM卡的数据   Lc: 文件标识长度  
//			  PKey: 密钥用途  VKey：密钥版本 
//			  
//
// 出口		：输出执行结果
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char Init_for_descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char PKey,unsigned char VKey,unsigned char* pDataOut )
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0


	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x1A;				// INS  
    sendApduData[2] = PKey;		        // P1 
    sendApduData[3] = VKey;			    // P2 
    sendApduData[4] = Lc;			    // lC 

	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 
	
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			__nop();
			return 0;
		}		 
		__nop();
		return 1;
		
		
	}
	return 	2; 

}



//*************************************************************************
 
 
// 函数名	：Descrypt
// 描述		：DES计算
// 入口		：pDataIn ：要发送给SAM卡要加密的数据   Lc: 要加密的数据长度  
//			  type: 最低位为0：表示加密，为1：表示MAC计算。次低位为0：表示无后续块，为1：表示有后续块。低第3位为0：表示MAC无初始值，为1：表示有初始值 
//			  Le: 要返回的加密数据结果长度
//
// 出口		：pDataOut ：由SAM卡返回执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************
unsigned char PSAM_Descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char type, unsigned char* pDataOut, unsigned char Le )
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
    sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xfa;				// INS 
    sendApduData[2] = type;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // LC 
 		  

	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 
	
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len-2);
		if((sw[0] == 0x90) &&(sw[1] == 0x00))
		{
			__nop();
			return 0;
		}
		if(sw[0] == 0x61)
		{
			 if(Sam_Get_Response( sw[1],pDataOut ,&len )   ==0)
				 return 0;			
		}
		__nop();
		return 1;
		
		
	}
	return 	2; 
}

//*************************************************************************
 
// 函数名	：Sam_ReadBinary
// 描述		：读二进制文件
// 入口		：Le : 要返回的数据长度	 file: 为短文件标识符
//			   
//			  
//
// 出口		：pDataOut ：由SAM卡返回的数据及执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************
unsigned char PSAM_ReadBinary( unsigned char file,unsigned char* pDataOut, unsigned int* Le )
{  
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xB0;				// INS 
    sendApduData[2] = 0x80|file;		// P1 --高3位为100 低5位为file
    sendApduData[3] = 0x00;			    // P2 --超始地址从0开始
    sendApduData[4] = *Le;			    // le 	
  
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,*Le);
		if((sw[0] == 0x90) &&(sw[1] == 0x00))
		{
			__nop();
			return 0;
		}		 
		__nop();
		return 1;
		
		
	}
	return 	2; 
	 

} 
 
//*************************************************************************
 
// 函数名	：Get_challenge
// 描述		：读取随机数
// 入口		：Le : 要返回的数据长度	 4字节或8字节
//			   
//			  
//
// 出口		：pDataOut ：由SAM卡返回的数据及执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************
unsigned char PSAM_Get_challenge(unsigned char Le,unsigned char* pDataOut)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x84;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Le;			    // le 


	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		
		if((sw[0] == 0x90) &&(sw[1] == 0x00))
		{
			memcpy(pDataOut,rxApduData,Le);
			__nop();
			return 0;
		}		 
		__nop();
		return 0xff;
		
		
	}
	return 	0xff; 
	
 

}
// 函数名	：PSAM_CreateFile
// 描述		：创建文件
// 入口		：pDataIn ：要发送给SAM卡的建立文件数据   Lc: 要建立的文件长度   type: 类型00建MF, 类型01建DF, 类型02建EF.    
// 			  status :  状态00正在建立, 状态01建立结束。
//
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char  PSAM_CreateFile(unsigned char *pDataIn, unsigned char Lc, unsigned char p1, unsigned char p2, unsigned char* pDataOut )
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xE0;				// INS 
    sendApduData[2] = p1;		        // P1 
    sendApduData[3] = p2;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			__nop();
			return 0;
		}		 
		__nop();
		return 1;
		
		
	}
	return 	2;  
 
}
// 函数名	：PSAMSetKey
// 描述		：增加或修改密钥
// 入口		：pDataIn ：要发送给SAM卡的密钥数据   Lc: 密钥数据长度   type: 类型01增加新密钥, 类型xx修改原来的密钥.  
//			 
//
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************

unsigned char PSAMSetKey(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char keyflag, unsigned char* pDataOut)
{

			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xD4;				// INS 
    sendApduData[2] = type;		        // P1 
    sendApduData[3] = keyflag;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			__nop();
			return 0;
		}		 
		__nop();
		return 1;
		
		
	}
	return 	2;  
}
// 函数名	：PSAM_UpdataBinary
// 描述		：写二进制文件
// 入口		：pDataIn ：要发送给SAM卡的文件标识   Lc: 文件标识长度  
//			  file: 为短文件标识符 
//			  
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态 
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************
unsigned char PSAM_UpdataBinary(unsigned char *pDataIn, unsigned char Lc, unsigned char file,unsigned char* pDataOut )
{
				unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xD6;				// INS 
    sendApduData[2] = 0x80|file;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		memcpy(pDataOut,rxApduData,len);
		if((sw[0] == 0x90) ||(sw[0] == 0x61))
		{
			__nop();
			return 0;
		}		 
		__nop();
		return 1;
		
		
	}
	return 	2; 
}
 
 
char PSAM_WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	unsigned int   sst=0;
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0xFA;				// CLA
    sendApduData[1] = 0x00;				// INS 
    sendApduData[2] = 0x01;		        // P1 
 
	//memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,3,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		//memcpy(pDataOut,rxApduData,len);
		if(len >=2)
		{
			*Out_Len = len-2;
			memcpy(pDataOut, rxApduData,*Out_Len);
		}
	 	return 0;
		
	}
	return 	2;  
	
}
					 
unsigned int PSAM_External_Authenticate(unsigned char Key_Flag,unsigned char *pDataIn,unsigned char *pDataOut)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	unsigned int   sst=0;
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x82;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = Key_Flag;			    // P2 
    sendApduData[4] = 0x08;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, 8); // 拷到发送缓冲区中	 
 
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+8,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		 
		sst=sw[0];
		sst=(sst<<8)|sw[1];
		if(sst==0x9000)
		{
			memcpy(pDataOut, rxApduData, len);
			return sst;
		}
		if(sw[0] == 0x63)
		{
			__nop();
			while(1);
		}
		__nop();
		return 1;
	}
	return 	0xff;  
	 
}



// 函数名	：PSAM_Format
// 描述		：格式化PSAM卡，把PSAM卡里面的全部内容删除。
// 入口		：pDataIn ：输入的数据为8个字节的FF   
// 			  Lc: 长度 
//
//
// 出口		：pDataOut ：由SAM卡返回的执行结果状态  
// 返回		：st :	结果为SAM_NG失败, SAM_OK成功
//*************************************************************************
unsigned int  PSAM_Format_1(unsigned char *pDataIn, unsigned char Lc, unsigned char* pDataOut)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	unsigned int   sst=0;
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x0E;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = 00;			    // le 
 
	//memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		//memcpy(pDataOut,rxApduData,len);
		sst=sw[0];
		sst=(sst<<8)|sw[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		}	 		 
		__nop();
		return 1;
		
		
	}
	return 	0xff;   
}
unsigned int  PSAM_Format_2(unsigned char *pDataIn, unsigned char Lc, unsigned char* pDataOut)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len,sst;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x0E;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		//memcpy(pDataOut,rxApduData,len);
		sst=sw[0];
		sst=(sst<<8)|sw[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		}
		return sst;		
	 
		
	}
	return 	0xff;   
}
// 函数名	：FM1208_Verify_Pin
// 描述		：PIN口令验证
// 入口		：File_Flag 口令标识
//            pDataIn ：输入的口令内容，即密码 
//			  Len 口令长度
//			  
// 出口		：无
// 返回		：成功返回sst=9000
//*************************************************************************
unsigned int PSAM_Verify_Pin(unsigned char File_Flag,unsigned char Lc, unsigned char *pDataIn)
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len,sst;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
 
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x20;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = File_Flag;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //把文件标识拷到发送缓冲区中	 

	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5+Lc,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		//memcpy(pDataOut,rxApduData,len);
		sst=sw[0];
		sst=(sst<<8)|sw[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		}
		return sst;		
	 
		
	}
	return 	0xff;
	
}
// 函数名	： MAC1
// 描述		： 把4个字节的MAC计算结果加在二进制文件指令数据的尾部
// 入口		： File_Flag 短文件标识  
//		       Len 要写入的数据长度
//		       pDataIn 要写入的数据
//		       LineCKey 线路保护密钥
//             Challenge 随机数
//             
// 出口     ： pDataOut	 已加4个字节的MAC值的指令数据
// 返回		： 无
//************************************************************************* 
void PSAM_MAC1(unsigned char File_Flag, unsigned char Len, unsigned char *pDataIn, unsigned char *LineCKey, unsigned char *Challenge,unsigned char *pDataOut,unsigned char *Le)
{
	unsigned char    MData[64];   
  	unsigned char    fout[4]={0,0,0,0};
 
	MData[0] = 0x04;			
	MData[1] = 0xD6;			    			
	MData[2] = 0x80|File_Flag;							 
	MData[3] = 0x00;			    									
	MData[4] = Len+4;
	 
	memcpy(&MData[5], pDataIn, Len);					  
    PBOC_MAC(&LineCKey[0], Challenge, MData, Len+5,fout);	
	
	memcpy(&MData[Len+5], fout, 4);	     //数据尾部加上4位的MAC
	
								
	Le[0]=MData[4];						 //输出数据的长度
	memcpy(pDataOut, &MData[5],Le[0]);	 //输出的数据
}

// 函数名	：PSAM_Format
// 描述		：格式化psam 卡，把卡里面MF下的全部内容删除。
// 入口		：无    
// 			  
//
//
// 出口		：无
// 返回		：成功返回sst=9000
//*************************************************************************

unsigned  int  PSAM_Format(void)
{
			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len,sst;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //清0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x0E;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = 0x00;			    // le 


	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData,5,rxApduData, &len);
	if (res == TDA8029_OK)
	{
		sw[0] = rxApduData[len-2];
		sw[1] = rxApduData[len-1];
		
	 
		sst=sw[0];
		sst=(sst<<8)|sw[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
		
		return sst;
		
	}
	return 	0XFF;
	
  

}
#if 0
//-------------------以下DATA数据，建在MF目录下的所有文件及密钥-----------------------------------------------------//
 
//创建MF文件,标识名为3F00//
//unsigned char static  MF_3F00[] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x31,0x50 ,0x41 ,0x59 ,0x2E ,0x53 ,0x59 ,0x53 ,0x2E ,0x44 ,0x44 ,0x46 ,0x30 ,0x31};
//指令说明：80（CLA）E0（INS）3F00（P1 P2 文件标识）0D（Lc）38（文件类型）FFFF（文件空间）F0（建立权限）F0（擦除权限）01（应用文件ID）FFFF（保留字）FFFFFFFFFF（DF名称）
unsigned char static  MF_3F00[] ={0x38,0xFF,0xFF,0xF0,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};//0x38,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x31,0x50 ,0x41 ,0x59 ,0x2E ,0x53 ,0x59 ,0x53 ,0x2E ,0x44 ,0x44 ,0x46 ,0x30 ,0x31};

//创建MF下的密钥文件,标识名为0000// //3F 00 B0 01 F0 FFFF
unsigned char static  MF_0000[] ={0x3f,0x00,0x50,0x01,0xF0,0xFF,0xFF};
//0、 PSAM MF 主控密钥 30（密钥标识）F0（使用权）F0（更改权）00（密钥版本号）00（算法标识：0-3des 1-des）  
//密钥0x30,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff

unsigned char static  MF_CCK_Key[] ={0x30,0xF0,0xF0,0x00,0x00, 0x30,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//1、 PSAM MF 维护密钥  35（密钥标识）F0（使用权）F0（更改权）00（密钥版本号）00（算法标识：0-3des 1-des）
unsigned char static  MF_CMK_Key[] ={0x35,0xF0,0xF0,0x00,0x00, 0x31,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

//2、 PSAM MF 线路保护密钥 80D401 00 0D 36 F0 F0 FF 33 FFFFFFFFFFFFFFFF
unsigned char static  MF_PlKey[] ={0x36,0xF0,0x02,0xFF,0x55,0x36,0x22,0x33,0x44,0x55,0x66,0x77,0x88,\
															0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//3、 PSAM MF 外部认证密钥
//外部认证 //  指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）
//   							 f9（密钥标识）F0（使用权）F0（更改权）AA（后续状态）0x55（错误计数器）FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF（密钥）
// 80D4 01 001539F0F0AA33 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

//unsigned char static  MF_ExKey[] ={0xf9,0xF0,0xF0,0xAA,0x55,0xf9,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
unsigned char static  MF_ExKey[] ={0x39,0xF0,0xF0,0xAA,0x55,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};\
                                                            //0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


////0、线路保护密钥// 80（CLA）D4（INS）01（P1）00（P2）0D（Lc）36（密钥标识）F0（使用权）F0（更改权）FF（默认）33（错误计数器）FFFFFFFFFFFFFFFF（密钥）
//unsigned char static  MF_LKey[] ={0x36,0xF0,0xF0,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

////1、外部认证 装载主密钥//  指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）
////   							39（密钥标识）F0（使用权）F0（更改权）AA（后续状态）88（错误计数器）FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF（密钥）
//// 
////2、装载PIN 解锁密钥// 80D401000D   80D401000D37F0F0FF551122334455667788   错误计算 55  密钥 1122334455667788
//unsigned char static  MF_PIN_Unblock[] ={0x37,0xF0,0xF0,0xFF,0x55,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

////3、装载PIN密钥// 80D40100083AF0EF4455123456  后续状态 44  错误计算 55   pin  123456 
//unsigned char static  MF_PINKey[] ={0x3A,0xF0,0xEF,0x44,0x55,0x12,0x34,0x56 };

////4、装载维护密钥// 重装密钥       80D4 0100 0D 38F0F0FF550102030405060708    错误计算 55  密钥0102030405060708
//unsigned char static  MF_AMK[] ={0x38,0xF0,0xF0,0xFF,0x55,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

 

//在MF下建立0015的公共信息文件//
unsigned char static  MF_0015[]={0x28,0x00,0x0e,0xF0,0xF0,0xFF,0xFF};

//在MF下建立0016的终端信息文件//
unsigned char static  MF_0016[]={0x28,0x00,0x06,0xF0,0xF0,0xFF,0xFF};


//在MF下建立0015的公共信息文件//
unsigned char static  MF_0015_flag[]={0x00,0x15};

//在MF下建立0016的终端信息文件//
unsigned char static  MF_0016_flag[]={0x00,0x16};





//-------------------以下DATA数据，建在DF目录下的所有文件及密钥-----------------------------------------------------//

//{0x3F,0x01,0x8F,0x95,0xF0,0xFF,0xFF}; 38036FF0F095FFFFA00000000386980701
//unsigned char static  MF_3F00[] ={0x38,0xFF,0xFF,0xF0,0xF0,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
//80E0 3F 01 0D 380520F0F095FFFF 4444463031
unsigned char static  DF_3F01[] ={0x38,0x05,0x6F,0xF0,0xF0,0x95,0xFF,0xFF,0x44,0x44,0x46,0x30,0x31}; //创建DF文件,即PBOC文件,标识名为3F01//
//{0x3f,0x00,0x50,0x01,0xF0,0xFF,0xFF};3F 01 8F 95 F0 FF FF
unsigned char static  DF_0000[] ={0x3F,0x01,0x8F ,0x95 ,0xF0 ,0xFF ,0xFF};             //创建DF下的EF密钥文件,标识名为0000//

 
//添加内部密钥
//    发送指令：80D4010015 34 F0 02 00 01 34343434343434343434343434343434
//	
//1、 装载内部 密钥   装载主密钥//34（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
unsigned char static 	DF_Internal_Key[]={0x34,0xF0,0x02,0x00,0x01,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34};

//2、文件线路保护密钥（这里的线路密钥和前面那个所保护的文件不同） 发送指令：80D401001536F002FF3336363636363636363636363636363636
    //指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）36（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）36363636363636363636363636363636（密钥）
unsigned char static 	DF_Pl_Key[]={0x36,0xF0,0x02,0xFF,0x77,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36};
//3、密钥线路保护密钥
unsigned char static 	DF_P_MAC2_Key[]={0x37,0xF0,0x02,0xFF,0x77,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//3、添加口令解锁密钥    发送指令：80D401001537F002FF3337373737373737373737373737373737    指令回复：9000（添加成功）
 //   指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）37（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）37373737373737373737373737373737（密钥）
unsigned char static 	DF_PinUn_Key[]={0x37,0xF0,0x02,0xFF,0x33,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//4、    添加口令重装密钥     发送指令：80D401001538F002FF3338383838383838383838383838383838     指令回复：9000（添加成功）
//    指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）38（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）38383838383838383838383838383838（密钥）
unsigned char static 	DF_PinRe_Key[]={0x38,0xF0,0x02,0xFF,0x33,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38};//37,0xF0,0x02,0xFF,0x33,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//5、装载PIN密钥// 80D40100083AF0EF4455123456  后续状态 44  错误计算 55   pin  123456 
unsigned char static  DF_PINKey[] ={0x3A,0xF0,0xEF,0x44,0x55,0x12,0x34,0x56 };

//6、    外部认证密钥     发送指令：80D401001539F002443339393939393939393939393939393939    指令回复：9000（添加成功）
//    指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）39（密钥标识）F0（使用权）02（更改权）44（后续状态）33（错误计数器）39393939393939393939393939393939（密钥）
unsigned char static  DF_ExKey[] ={0x39,0xF0,0x02,0x44,0x77,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39};

//7、 添加修改透支限额密钥 发送指令：80 D4 01 01 15 3C F00201003C013C013C013C013C013C013C013C01	 
unsigned char static 	DF_ODKey[]={0x3C ,0xF0,0x02,0x01,0x00,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01};

//8、装载消费密钥//	 80 D4 01 01 15 3E F00200013E013E013E013E013E013E013E013E01                    
unsigned char static 	DF_DPKey[]={0x3E ,0xF0,0x02,0x00,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01};

//9、用作圈存密钥 //      发送指令：80 D4 01 01 15 3F F0 02 00 01 3F013F013F013F013F013F013F013F01         				
unsigned char static 	Credit_For_Load_KEY[]={0x3F,0xF0,0x02,0x00,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01};
//10、用作圈提密钥 //80 D4 01 01 15 3D F0 02 01003D013D013D013D013D013D013D013D01
unsigned char static 	DCredit_For_Load_KEY[]={0x3D,0xF0,0x02,0x01,0x00,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01};
 
//11、des加密密钥 // 30（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
unsigned char static 	DF_DES_E_KEY[]={0x30,0xF0,0x02,0x00,0x01,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};

//12、des解密密钥 // 31（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
unsigned char static 	DF_DES_D_KEY[]={0x31,0xF0,0x02,0x00,0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31};
 
//13、desmac密钥 // 32（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
unsigned char static 	DF_DES_MAC_KEY[]={0x32,0xF0,0x02,0x00,0x01,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32};
 
		

 //  创建15号文件（二进制文件）    发送指令：80 E0 00 15 07 A8001EF0F0FFFF    指令回复：9000（创建成功）
//    指令说明：80（CLA）E0（INS）0015（P1 P2 文件标识）07（Lc）A8（明文MAC 28(二进制文件高位变1)–>A8）001E（文件空间）F0（读权限）F0（增加权限）FF（默认FF）FF（默认FF）
//注：28–>A8 28=00101000 高位变1即：10101000=A8（明文+MAC校验）
unsigned char static  DF_0015[]={0x28,0x00,0x1E,0xF0,0xF0,0xFF,0xFF};

//    创建17号文件（二进制文件）    发送指令：80E0001707 2805DCF0F0FFFF    指令回复：9000（创建成功）
//指令说明：80（CLA）E0（INS）0017（P1 P2 文件标识）07（Lc）28（二进制文件）05DC（文件空间）F0（读权限）F0（添加权限）FF（默认FF）FF（默认FF）
unsigned char static  DF_0016[]={0x28,0x00,0x27,0xF0,0xF0,0xFF,0xFF};

//    创建17号文件（二进制文件）    发送指令：80E0001707 2805DCF0F0FFFF    指令回复：9000（创建成功）
//指令说明：80（CLA）E0（INS）0017（P1 P2 文件标识）07（Lc）28（二进制文件）05DC（文件空间）F0（读权限）F0（添加权限）FF（默认FF）FF（默认FF）
unsigned char static  DF_0017[]={0x28,0x00,0x20,0xF0,0xF0,0xFF,0xFF};

 //创建18号文件（循环文件）    交易明细文件 发送指令：80E0001807 2E0A17F0EFFFFF
//80（CLA）E0（INS）0018（P1 P2 文件标识）07（Lc）2E（循环文件）0A17（文件空间）F0（读权限）EF（增加权限）FF（默认FF）FF（默认FF）
unsigned char static  DF_0018[]={0x2E,0x0A,0x17,0xF0,0xEF,0xFF,0xFF};

//创建钱包文件(电子存折)  发送指令：80 E0 00 01 07 2F0208F100FF18    指令回复：9000（创建成功）
//    指令说明：80（CLA）E0（INS）0001（P1 P2 文件标识）07（Lc）2F（PBOC ED/EP）0208（默认0208）F1（使用权）00（保留00）FF（默认FF）18（交易明细文件短标识）
unsigned char static  DF_0001[]={0x2F,0x02,0x08,0xF1,0x00,0xFF,0x18};

//创建钱包文件(电子钱包)     发送指令：80 E0 00 02 07 2F 02 08 F0 00 FF 18    指令回复：9000（创建成功）
//指令说明：80（CLA）E0（INS）0002（P1 P2 文件标识）07（Lc）2F（PBOC ED/EP）0208（默认0208）F0（使用权）00（保留00）FF（默认FF）18（交易明细文件短标识）
unsigned char static  DF_0002[]={0x2F,0x02,0x08,0xF0,0x00,0xFF,0x18};

 

	
unsigned char static  PosNum[]={'L','L',0x30,0x31,0x32,0x33};   //终端机编号，即本机编号由发卡者自行定义。		
unsigned char static  MFflag[]={0x3F,0x00};						//MF的文件标识//
//unsigned char static  MFPkey[]={0x12,0x34,0x56,0x78}; 			//PIN密码//

unsigned char static  PublicFile_3F01[]={0x02,0xA0,0x00,0x00,0x00,0x03,0x00,0x00,0x01,0x20,0x16,0x08,0x15,0x00,0x00,0x00,0x01,0x20,0x16,0x10,0x01,0x20,0x18,0x09,0x31};//全国密钥管理中心公共信息文件
unsigned char static  Pos_Transaction_Num[]={0x00,0x00,0x00,0x00,}; //终端机所存的交易序号

unsigned char WorkInt;
unsigned char Cmd_Len;
unsigned char   Rec_Buf[64];   
unsigned char   revbuff[64];
extern unsigned int gRepLen;
extern unsigned char File_3F01[] ; 
extern unsigned char File_3F00[] ;
extern unsigned char File_0015[];
extern unsigned char File_0016[];
extern unsigned char File_0017[];
extern unsigned char File_0018[];
void PSAM_SendCard(void)
{
		 unsigned  int st; 
	unsigned char tt[2];	
	 unsigned char SAM_ATR[16];	
	 unsigned char SendData[16];
	 unsigned char psam_Challenge_Num[8] = {0};

	  while(1)
	  {
	       //st=PSAM_CreateFile( MF_3F00, sizeof(MF_3F00), Type_MF, Start, revbuff );	//创建MF
		  
//		   st=PSAM_Select_File(File_3F00, 2, 0,revbuff, &gRepLen);// 0x28 );	//选择SAM卡中PBOC的3F00文件// 28
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值
//			  UART_Send_Str("该文件是否已经 创 建 ?请先格式化SAM卡再操作");
//			  while(1);
//		   }
//			UART_Send_Str("\r\nopen 3f 00 ok\r\n");
			#if 1
		   st=PSAM_CreateFile( MF_3F00, sizeof(MF_3F00), 0x3f, 0x00, revbuff );
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值
			  UART_Send_Str("该文件是否已经 创 建 ?请先格式化SAM卡再操作");
			   st=PSAM_Select_File(File_3F00, 2, 0,revbuff, &gRepLen);// 0x28 );	//选择SAM卡中PBOC的3F00文件// 28
				if(st!=SAM_OK)
				{
					UART_Send_Str("错误代码:");
					UART_Put_Num(revbuff,2);	//返回2个字节的状态值
					UART_Send_Str("该文件是否已经 创 建 ?请先格式化SAM卡再操作");
					while(1);
				}	 
			  //while(1);
		   }
		   __nop();
		   UART_Send_Str("MF文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
			#endif
		   //------------------------------------------------------------------------------------------------------
		   #if 1
		   st=PSAM_CreateFile( MF_0000, sizeof(MF_0000), Type_EF, Start, revbuff );	//创建MF下的EF密钥文件
		   if(st!=SAM_OK)
		   {	  
			  UART_Send_Str("创建MF下的EF密钥文件 错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			   while(1);
			  continue;
		   }
		   __nop();
		   UART_Send_Str("MF下的密钥文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
		  #endif
			//0、-----------------Write Key 命令报文编码的方式一 -----------------------------------------------------------
	 	   st=PSAMSetKey(MF_CCK_Key,sizeof(MF_CCK_Key), 0x01,0X30, revbuff );   //装载MF下的 0x30主密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue; 	  
	
		   } 
//		 
		   #if 1
		   //1、------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_CMK_Key,sizeof(MF_CMK_Key), 0x01,0x35, revbuff );   //装载MF下的维护密钥 0x36
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue; 	  
	
		   }
		   __nop();
		   UART_Send_Str("维护密钥 装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值		  
			

		   //2、------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_PlKey,sizeof(MF_PlKey), 0x01,0x36, revbuff );   //装载MF下的 线路保护密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("线路保护密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值


		   //3、------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_ExKey,sizeof(MF_ExKey), 0x01,0X39, revbuff );   //装载MF下外部认证密钥 0X39
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("外部认证密钥 错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("外部认证密钥 装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
 		  #endif
		  

		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( MF_0015, sizeof(MF_0015), 0x00, 0x15, revbuff );	//创建MF下0015的公共信息文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			   while(1);
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("0015公共信息文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
		 
		   
		  
//		   st=PSAM_Select_File(File_3F00, 2, 0,revbuff, &gRepLen);// 0x28 );
//		    if(st!=SAM_OK)								 
//		    {
//				printf("error open 3f00\r\n");
//				while(1);
//			}
		 
//		   st=PSAM_Select_File(File_0015, 2, 0,revbuff, &gRepLen);// 0x28 );
//			if(st!=SAM_OK)								 
//		    {	
//				printf("error open 3f00\r\n");
//				
//			}
//			printf("open ok\r\n");
//			  #if 0
//		    st=PSAM_Get_challenge(4,revbuff);		       //从SAM卡里取一组随机数//    
//		    if(st!=SAM_OK)								 
//		    {
//			    UART_Send_Str("SAM卡读取随机码错误:");
//				UART_Put_Num(revbuff,2);   //显示错误代码
//				UART_Send_Enter();			   					            
//			    continue;		
//		    }
// 			memcpy(psam_Challenge_Num, revbuff, 4);     //临时保存随机数
//			memset(&psam_Challenge_Num[4],0, 4);     // 
//			#endif
		   memset(SendData, 0x00, 16);        //清0
		   //memcpy(SendData, &SAM_ATR[6], 10); //PSAM卡序列号
		   SendData[9]=0x01;
			SendData[10]=0x01;  		  //PSAM卡版本号
		   SendData[11]=0x06; 			      //密钥卡类型
		   SendData[12]=0x55;  		          //自定义FCI数据
		   SendData[13]=0x66; 			      //自定义FCI数据
//		    PSAM_MAC1(0x15 , 14, SendData, &MF_PlKey[5],psam_Challenge_Num,revbuff,tt);
//			printf("mac后的数据");
//			UART_Put_Num(revbuff,tt[0]);   //显示错误代码
//			UART_Send_Enter();	
		 
			//FM1208_Update_Binary(0x15,revbuff, tt[0] ));    //执行把加线路保护的数据写入0015中。
			 st=PSAM_UpdataBinary(SendData, 14, 0x15,revbuff );
			//st=PSAM_UpdataBinary(revbuff,tt[0],0x15,Rec_Buf );// 0x80|file
			if(st!=SAM_OK)
			{
				UART_Send_Str(" \r\n错误:\r\n");
				//UART_Put_Num(Rec_Buf,2);   //显示错误代码
				while(1);
			}
			UART_Send_Str("0015信息写入成功!");
			UART_Send_Enter();
 
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( MF_0016, sizeof(MF_0016), 0x00, 0x16, revbuff);	//创建MF下建立MF_0016终端信息文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
   			  while(1);

			  continue;	  
	
		   }  
		   #if 0
		    st=PSAM_Get_challenge(4,revbuff);		       //从SAM卡里取一组随机数//    
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM卡读取随机码错误:");
				UART_Put_Num(revbuff,2);   //显示错误代码
				UART_Send_Enter();			   					            
			    continue;		
		    }
 			memcpy(psam_Challenge_Num, revbuff, 4);     //临时保存随机数
			memset(&psam_Challenge_Num[4],0, 4);     // 
		 	 
		    PSAM_MAC1(0x16 , 6, PosNum, &MF_PlKey[5],psam_Challenge_Num,revbuff,tt);
			printf("mac后的数据\r\n");
			UART_Put_Num(revbuff,tt[0]);   //显示错误代码
			UART_Send_Enter();	
			#endif
//			st=PSAM_UpdataBinary(PosNum,6,0x16,Rec_Buf );
//			    //执行把加线路保护的数据写入0016中。
//			//st=PSAM_UpdataBinary(revbuff,tt[0],0x16,Rec_Buf );// 0x80|file
//			if(st!=SAM_OK)
//			{
//				UART_Send_Str(" \r\n write 0016 错误:\r\n");
//				//UART_Put_Num(Rec_Buf,2);   //显示错误代码
//				while(1);
//			} 
		   
		    st=PSAM_UpdataBinary(PosNum, 6, 0x16,revbuff );
	 	   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			   while(1);
			  continue;
		
		   }
		   UART_Send_Str("成功写入0016终端机编号:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
		  // while(1);
//		   
//		   UART_Send_Str("终端机信息文件创建成功:");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

		   //------------------------------------------------------------------------------------------------------
	
//		   memset(SendData, 0x00, 16);        //清0
//		   memcpy(SendData, &SAM_ATR[6], 10); //PSAM卡序列号
//		   SendData[10]=SAM_ATR[4];  		  //PSAM卡版本号
//		   SendData[11]=0x06; 			      //密钥卡类型
//		   SendData[12]=0x55;  		          //自定义FCI数据
//		   SendData[13]=0x66; 			      //自定义FCI数据
//			  

//		   st=PSAM_Select_File(MF_0015_flag, 2, 0,revbuff,&gRepLen);
//		
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//   			  while(1);

//			  continue;
//		
//		   }
//		   UART_Send_Str(" 0015的公共信息打开成功:");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
//		   st=PSAM_UpdataBinary(SendData, 14, 0x15,revbuff );
//	 	   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//			   while(1);
//			  continue;
//		
//		   }
//		   UART_Send_Str("打开15文件成功");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
//    	           st=PSAM_Select_File(MF_0016_flag, 2, 0,revbuff,&gRepLen);
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//			  continue;
//		
//		   }
//		   UART_Send_Str("打开16文件成功");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

//		   //------------------------------------------------------------------------------------------------------


//		
 
		#if 1
		   //------------------------------------------------------------------------------------------------------
   		   st=PSAM_CreateFile( DF_3F01, sizeof(DF_3F01), 0x3f, 0x01, revbuff);	//创建MF下的DF文件为3F01
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
  			  while(1);

			   continue;	  
	
		   }
		   UART_Send_Str("3F01文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
	
			
		 // PSAM_Select_File(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char* pDataOut, unsigned int *Le );
			st=PSAM_Select_File(File_3F01, 2, 0,revbuff,&gRepLen);
		    if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  while(1);
			   continue;	  
	
		   }
		   UART_Send_Str("3F01文件打开成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
		   //------------------------------------------------------------------------------------------------------
   		   st=PSAM_CreateFile( DF_0000, sizeof(DF_0000), 0x00, 0x00, revbuff);	////创建DF下的EF密钥文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的密钥文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
		 
//1、 装载内部 密钥   装载主密钥//34（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（错误计数器）DF_Internal_Key[]={0x34,
		    st=PSAMSetKey(DF_Internal_Key,sizeof(DF_Internal_Key), 0x01,0x34,revbuff );   //装载DF下的主控密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的主密钥01装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////2、路保护密钥（这里的线路密钥和前面那个所保护的文件不同） 发送指令：80D401001536F002FF3336363636363636363636363636363636
//   //指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）36（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）36363636363636363636363636363636（密钥）
// 	DF_Pl_Key[]={0x36,
		   st=PSAMSetKey(DF_Pl_Key,sizeof(DF_Pl_Key), 0x01,0x36,revbuff );   //装载DF下的线路保护密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的线路保护密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////3、添加口令解锁密钥    发送指令：80D401001537F002FF3337373737373737373737373737373737    指令回复：9000（添加成功）
// //   指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）37（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）37373737373737373737373737373737（密钥）
	      st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x37,revbuff );   //装载DF下的口令解锁密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的口令解锁密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////4、    添加口令重装密钥     发送指令：80D401001538F002FF3338383838383838383838383838383838     指令回复：9000（添加成功）
////    指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）38（密钥标识）F0（使用权）02（更改权）FF（默认FF）33（错误计数器）38383838383838383838383838383838（密钥）
		    st=PSAMSetKey(DF_PinRe_Key,sizeof(DF_PinRe_Key), 0x01,0x38,revbuff );   //装载DF下的口令重装密钥  
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的口令重装密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////5、装载PIN密钥// 80D40100083AF0EF4455123456  后续状态 44  错误计算 55   pin  123456 
//  DF_PINKey[] ={0x3A,
			st=PSAMSetKey(DF_PINKey,sizeof(DF_PINKey), 0x01,0x3A,revbuff );   //装载DF下的口令PIN密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的PIN密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////6、    外部认证密钥     发送指令：80D401001539F002443339393939393939393939393939393939    指令回复：9000（添加成功）
////    指令说明：80（CLA）D4（INS）01（P1）00（P2）15（Lc）39（密钥标识）F0（使用权）02（更改权）44（后续状态）33（错误计数器）39393939393939393939393939393939（密钥）
//  DF_ExKey[] ={0x39,
    st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x39,revbuff );   //装载DF下的外部认证密钥  
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的线路保护密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////7、 添加修改透支限额密钥 发送指令：80 D4 01 01 15 3C F00201003C013C013C013C013C013C013C013C01	 
// 	DF_ODKey[]={0x3C
    st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x3C,revbuff );   //装载DF下的口令解锁密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的线路保护密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////8、装载消费密钥//	 80 D4 01 01 15 3E F00200013E013E013E013E013E013E013E013E01                    
// 	DF_DPKey[]={0x3E 
    st=PSAMSetKey(DF_DPKey,sizeof(DF_DPKey), 0x01,0x3E,revbuff );   //装载DF下的口令消费密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的消费密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////9、用作圈存密钥 //      发送指令：80 D4 01 01 15 3F F0 02 00 01 3F013F013F013F013F013F013F013F01         				
// 	Credit_For_Load_KEY[]={0x3F,
		       st=PSAMSetKey(Credit_For_Load_KEY,sizeof(Credit_For_Load_KEY), 0x01,0x3F,revbuff );   //装载DF下的圈存密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的圈存密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

////10、用作圈提密钥 	DCredit_For_Load_KEY[]={0x3D,
// 
		      st=PSAMSetKey(DCredit_For_Load_KEY,sizeof(DCredit_For_Load_KEY), 0x01,0x3D,revbuff );   //装载DF下的圈提密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的圈提密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值


//		  //11、des加密密钥 // 30（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
//unsigned char static 	DF_DES_E_KEY[]={0x30,0xF0,0x02,0x00,0x01,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
		   	st=PSAMSetKey(DF_DES_E_KEY,sizeof(DF_DES_E_KEY), 0x01,0x30,revbuff );   //装载DF下的des加密密钥 
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的des加密密钥 装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值


////12、des解密密钥 // 31（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
//unsigned char static 	DF_DES_D_KEY[]={0x31,0xF0,0x02,0x00,0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31};
	      st=PSAMSetKey(DF_DES_D_KEY,sizeof(DF_DES_D_KEY), 0x01,0x31,revbuff );   //装载DF下的des解密密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的des解密密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
 
////13、desmac密钥 // 32（密钥标识）F0（使用权）02（更改权）00（密钥版本）01（算法标识）
	//unsigned char static 	DF_DES_MAC_KEY[]={0x32,0xF0,0x02,0x00,0x01,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32};
	      st=PSAMSetKey(DF_DES_MAC_KEY,sizeof(DF_DES_MAC_KEY), 0x01,0x32,revbuff );   //装载DF下的desmac密钥
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下的desmac密钥装载成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
 
 
			 
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( DF_0015, sizeof(DF_0015), 0x00, 0x15, revbuff );	//创建DF下0015 公共应用基本数据文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("0015公共应用基本数据文件:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值


			st=PSAM_CreateFile( DF_0016,sizeof(DF_0016), 0x00, 0x16, revbuff );	//创建DF下0016 持卡人基本数据文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("0016持卡人基本数据文件:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值


			st=PSAM_CreateFile( DF_0017,sizeof(DF_0017), 0x00, 0x17, revbuff );	//创建DF下0017持卡人基本数据文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF下0017 基本数据文件:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( DF_0018, sizeof(DF_0018), 0x00, 0x18, revbuff);	//创建DF下建立0018终端交易序号文件
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;	  
	
		   }
		   UART_Send_Str("0018终端交易序号文件创建成功:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值
			#endif //while(1);
		   
		//   st=PSAM_Select_File(File_3F01, 2, 0,revbuff, &gRepLen);// 0x28 );
		//   st=PSAM_Select_File(File_0017, 2, 0,revbuff, &gRepLen);
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_UpdataBinary(PublicFile_3F01, sizeof(PublicFile_3F01), 0x17,revbuff );
	 	   if(st!=SAM_OK)
		   {
			  UART_Send_Str("错误代码:");
			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
			  continue;
		
		   }		    
		   UART_Send_Str("成功写入全国密钥管理中心公共信息内容:");
		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

		//   st=PSAM_Select_File(File_0018, 2, 0,revbuff, &gRepLen);
		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_UpdataBinary(Pos_Transaction_Num, sizeof(Pos_Transaction_Num), 0x18,revbuff );  
//	 	   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//			  continue;
//		
//		   }
//		    
//		   UART_Send_Str("成功初始化终端交易序号:");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

			

//		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_CreateFile( DF_3F01, 2, Type_DF, End, revbuff );	//结束创建DF
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//			  continue;
//		
//		   }
//		   
//		   UART_Send_Str("DF文件创建结束:");
//		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值



//		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_CreateFile( MFflag, sizeof(MFflag), Type_MF, End, revbuff );	//结束创建MF
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("错误代码:");
//			  UART_Put_Num(revbuff,2);	//返回2个字节的状态值	  
//			  continue;
//		
//		   }
//		   
 		   UART_Send_Str("创建结束:");
 		   UART_Put_Num(revbuff,2);	//返回2个字节的状态值

		 
			psam_delay_10ms(200);

			while(1);
	   }

}
#endif
 