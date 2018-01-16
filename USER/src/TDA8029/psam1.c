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
unsigned char PSAMInit(unsigned char* pDataOut)   //PSAM����ʼ��  ������ȷ����Ӧ��Ϊ:3B 6C 00 02 13 02 86 38 18 43 56 07 1E 10 2B 23     
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
 
    return 1;        //��λʧ��   	  
}

//*************************************************************************
 
// ������	��Sam_Selsect_File
// ����		��ѡ��򿪵�ǰ�ļ�
// ���		��pDataIn ��Ҫ���͸�SAM�����ļ���ʶ   Lc: �ļ���ʶ����  
//			  Le : Ҫ���ص����ݳ���	 type: ����00 ���ļ���ʶ��ѡ��MF��DF, ������Ϊ02 ��ѡ��EF 
//			  ������Ϊ04 ���ļ�����ѡ��
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char Sam_Get_Response(  unsigned char Lc, unsigned char* pDataOut,unsigned int *Le )  
{

	unsigned char st,res;
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
		unsigned int len;
	memset(sendApduData, 0x00, MAXRLEN);  //��0

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
//	if(SendCmd(sam_snd_buff,  Lc, pDataOut, Le )!=0x9000)	 //��������жϽ��
//	st=SAM_NG;	   

//	else
//	st=SAM_OK;	   

//	return 	st;

}
//*************************************************************************
 
// ������	��PSAM_Select_File
// ����		��ѡ��򿪵�ǰ�ļ�
// ���		��pDataIn ��Ҫ���͸�SAM�����ļ���ʶ   Lc: �ļ���ʶ����  
//			  Le : Ҫ���ص����ݳ���	 type: ����00 ���ļ���ʶ��ѡ��MF��DF, ������Ϊ02 ��ѡ��EF 
//			  ������Ϊ04 ���ļ�����ѡ��
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char PSAM_Select_File(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char* pDataOut,unsigned int *Le )  
{

	unsigned char st,res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0


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
// ������	��Init_SAM_For_Purchase
// ����		��MAC1�����ʼ������������
// ���		��pDataIn ���û�����α�������������š���������   Lc: ���ݳ��ȣ�����Ϊ14H+18H������ο�SAM����COS�ֲ�  
 		  
//
// ����		��pDataOut ��PSAM�ն˽�����š�MAC1�� 
// ����		�����ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char Init_SAM_For_Purchase(unsigned char *pDataIn, unsigned char Lc,  unsigned char* pDataOut)
{
			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
 

	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x70;				// INS  
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // lC 

	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 
	
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
// ������	��Credit_SAM_For_Purchase
// ����		�����û����ṩ��MAC2����SAM������У�飬��������
// ���		��pDataIn ��MAC2����   Lc: ���ݳ��ȹ涨4���ֽ� 
//			  
//			
//
// ����		��pDataOut ����SAM�����صļ��ܺ�����ݼ�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char Credit_SAM_For_Purchase(unsigned char *pDataIn, unsigned char* pDataOut)
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
 

	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x72;				// INS  
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = 0x04;			    // lC 

	memcpy(&sendApduData[5], pDataIn, 0x04); //���ļ���ʶ�������ͻ�������	 
	
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
 
// ������	��Init_for_descrypt
// ����		��DES�����ʼ��
// ���		��pDataIn ��Ҫ���͸�SAM��������   Lc: �ļ���ʶ����  
//			  PKey: ��Կ��;  VKey����Կ�汾 
//			  
//
// ����		�����ִ�н��
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char Init_for_descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char PKey,unsigned char VKey,unsigned char* pDataOut )
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0


	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x1A;				// INS  
    sendApduData[2] = PKey;		        // P1 
    sendApduData[3] = VKey;			    // P2 
    sendApduData[4] = Lc;			    // lC 

	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 
	
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
 
 
// ������	��Descrypt
// ����		��DES����
// ���		��pDataIn ��Ҫ���͸�SAM��Ҫ���ܵ�����   Lc: Ҫ���ܵ����ݳ���  
//			  type: ���λΪ0����ʾ���ܣ�Ϊ1����ʾMAC���㡣�ε�λΪ0����ʾ�޺����飬Ϊ1����ʾ�к����顣�͵�3λΪ0����ʾMAC�޳�ʼֵ��Ϊ1����ʾ�г�ʼֵ 
//			  Le: Ҫ���صļ������ݽ������
//
// ����		��pDataOut ����SAM������ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************
unsigned char PSAM_Descrypt(unsigned char *pDataIn, unsigned char Lc, unsigned char type, unsigned char* pDataOut, unsigned char Le )
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
    sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xfa;				// INS 
    sendApduData[2] = type;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // LC 
 		  

	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 
	
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
 
// ������	��Sam_ReadBinary
// ����		�����������ļ�
// ���		��Le : Ҫ���ص����ݳ���	 file: Ϊ���ļ���ʶ��
//			   
//			  
//
// ����		��pDataOut ����SAM�����ص����ݼ�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************
unsigned char PSAM_ReadBinary( unsigned char file,unsigned char* pDataOut, unsigned int* Le )
{  
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xB0;				// INS 
    sendApduData[2] = 0x80|file;		// P1 --��3λΪ100 ��5λΪfile
    sendApduData[3] = 0x00;			    // P2 --��ʼ��ַ��0��ʼ
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
 
// ������	��Get_challenge
// ����		����ȡ�����
// ���		��Le : Ҫ���ص����ݳ���	 4�ֽڻ�8�ֽ�
//			   
//			  
//
// ����		��pDataOut ����SAM�����ص����ݼ�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************
unsigned char PSAM_Get_challenge(unsigned char Le,unsigned char* pDataOut)
{
	unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
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
// ������	��PSAM_CreateFile
// ����		�������ļ�
// ���		��pDataIn ��Ҫ���͸�SAM���Ľ����ļ�����   Lc: Ҫ�������ļ�����   type: ����00��MF, ����01��DF, ����02��EF.    
// 			  status :  ״̬00���ڽ���, ״̬01����������
//
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char  PSAM_CreateFile(unsigned char *pDataIn, unsigned char Lc, unsigned char p1, unsigned char p2, unsigned char* pDataOut )
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xE0;				// INS 
    sendApduData[2] = p1;		        // P1 
    sendApduData[3] = p2;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
// ������	��PSAMSetKey
// ����		�����ӻ��޸���Կ
// ���		��pDataIn ��Ҫ���͸�SAM������Կ����   Lc: ��Կ���ݳ���   type: ����01��������Կ, ����xx�޸�ԭ������Կ.  
//			 
//
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************

unsigned char PSAMSetKey(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char keyflag, unsigned char* pDataOut)
{

			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0xD4;				// INS 
    sendApduData[2] = type;		        // P1 
    sendApduData[3] = keyflag;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
// ������	��PSAM_UpdataBinary
// ����		��д�������ļ�
// ���		��pDataIn ��Ҫ���͸�SAM�����ļ���ʶ   Lc: �ļ���ʶ����  
//			  file: Ϊ���ļ���ʶ�� 
//			  
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬ 
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
//*************************************************************************
unsigned char PSAM_UpdataBinary(unsigned char *pDataIn, unsigned char Lc, unsigned char file,unsigned char* pDataOut )
{
				unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0xD6;				// INS 
    sendApduData[2] = 0x80|file;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0xFA;				// CLA
    sendApduData[1] = 0x00;				// INS 
    sendApduData[2] = 0x01;		        // P1 
 
	//memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x82;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = Key_Flag;			    // P2 
    sendApduData[4] = 0x08;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, 8); // �������ͻ�������	 
 
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



// ������	��PSAM_Format
// ����		����ʽ��PSAM������PSAM�������ȫ������ɾ����
// ���		��pDataIn �����������Ϊ8���ֽڵ�FF   
// 			  Lc: ���� 
//
//
// ����		��pDataOut ����SAM�����ص�ִ�н��״̬  
// ����		��st :	���ΪSAM_NGʧ��, SAM_OK�ɹ�
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
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x0E;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = 00;			    // le 
 
	//memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
	sendApduData[0] = 0x80;				// CLA
    sendApduData[1] = 0x0E;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = 0x00;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
// ������	��FM1208_Verify_Pin
// ����		��PIN������֤
// ���		��File_Flag �����ʶ
//            pDataIn ������Ŀ������ݣ������� 
//			  Len �����
//			  
// ����		����
// ����		���ɹ�����sst=9000
//*************************************************************************
unsigned int PSAM_Verify_Pin(unsigned char File_Flag,unsigned char Lc, unsigned char *pDataIn)
{
		unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len,sst;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
 
	sendApduData[0] = 0x00;				// CLA
    sendApduData[1] = 0x20;				// INS 
    sendApduData[2] = 0x00;		        // P1 
    sendApduData[3] = File_Flag;			    // P2 
    sendApduData[4] = Lc;			    // le 
 
	memcpy(&sendApduData[5], pDataIn, Lc); //���ļ���ʶ�������ͻ�������	 

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
// ������	�� MAC1
// ����		�� ��4���ֽڵ�MAC���������ڶ������ļ�ָ�����ݵ�β��
// ���		�� File_Flag ���ļ���ʶ  
//		       Len Ҫд������ݳ���
//		       pDataIn Ҫд�������
//		       LineCKey ��·������Կ
//             Challenge �����
//             
// ����     �� pDataOut	 �Ѽ�4���ֽڵ�MACֵ��ָ������
// ����		�� ��
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
	
	memcpy(&MData[Len+5], fout, 4);	     //����β������4λ��MAC
	
								
	Le[0]=MData[4];						 //������ݵĳ���
	memcpy(pDataOut, &MData[5],Le[0]);	 //���������
}

// ������	��PSAM_Format
// ����		����ʽ��psam �����ѿ�����MF�µ�ȫ������ɾ����
// ���		����    
// 			  
//
//
// ����		����
// ����		���ɹ�����sst=9000
//*************************************************************************

unsigned  int  PSAM_Format(void)
{
			unsigned char res;
	unsigned char sw[2];
    unsigned char  sendApduData[MAXRLEN];
	unsigned char  rxApduData[MAXRLEN];
	unsigned int len,sst;
	
	len = sizeof(rxApduData);
	memset(sendApduData, 0x00, MAXRLEN);  //��0
	
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
//-------------------����DATA���ݣ�����MFĿ¼�µ������ļ�����Կ-----------------------------------------------------//
 
//����MF�ļ�,��ʶ��Ϊ3F00//
//unsigned char static  MF_3F00[] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x31,0x50 ,0x41 ,0x59 ,0x2E ,0x53 ,0x59 ,0x53 ,0x2E ,0x44 ,0x44 ,0x46 ,0x30 ,0x31};
//ָ��˵����80��CLA��E0��INS��3F00��P1 P2 �ļ���ʶ��0D��Lc��38���ļ����ͣ�FFFF���ļ��ռ䣩F0������Ȩ�ޣ�F0������Ȩ�ޣ�01��Ӧ���ļ�ID��FFFF�������֣�FFFFFFFFFF��DF���ƣ�
unsigned char static  MF_3F00[] ={0x38,0xFF,0xFF,0xF0,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};//0x38,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x31,0x50 ,0x41 ,0x59 ,0x2E ,0x53 ,0x59 ,0x53 ,0x2E ,0x44 ,0x44 ,0x46 ,0x30 ,0x31};

//����MF�µ���Կ�ļ�,��ʶ��Ϊ0000// //3F 00 B0 01 F0 FFFF
unsigned char static  MF_0000[] ={0x3f,0x00,0x50,0x01,0xF0,0xFF,0xFF};
//0�� PSAM MF ������Կ 30����Կ��ʶ��F0��ʹ��Ȩ��F0������Ȩ��00����Կ�汾�ţ�00���㷨��ʶ��0-3des 1-des��  
//��Կ0x30,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff

unsigned char static  MF_CCK_Key[] ={0x30,0xF0,0xF0,0x00,0x00, 0x30,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

//1�� PSAM MF ά����Կ  35����Կ��ʶ��F0��ʹ��Ȩ��F0������Ȩ��00����Կ�汾�ţ�00���㷨��ʶ��0-3des 1-des��
unsigned char static  MF_CMK_Key[] ={0x35,0xF0,0xF0,0x00,0x00, 0x31,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

//2�� PSAM MF ��·������Կ 80D401 00 0D 36 F0 F0 FF 33 FFFFFFFFFFFFFFFF
unsigned char static  MF_PlKey[] ={0x36,0xF0,0x02,0xFF,0x55,0x36,0x22,0x33,0x44,0x55,0x66,0x77,0x88,\
															0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//3�� PSAM MF �ⲿ��֤��Կ
//�ⲿ��֤ //  ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��
//   							 f9����Կ��ʶ��F0��ʹ��Ȩ��F0������Ȩ��AA������״̬��0x55�������������FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF����Կ��
// 80D4 01 001539F0F0AA33 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

//unsigned char static  MF_ExKey[] ={0xf9,0xF0,0xF0,0xAA,0x55,0xf9,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
unsigned char static  MF_ExKey[] ={0x39,0xF0,0xF0,0xAA,0x55,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};\
                                                            //0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


////0����·������Կ// 80��CLA��D4��INS��01��P1��00��P2��0D��Lc��36����Կ��ʶ��F0��ʹ��Ȩ��F0������Ȩ��FF��Ĭ�ϣ�33�������������FFFFFFFFFFFFFFFF����Կ��
//unsigned char static  MF_LKey[] ={0x36,0xF0,0xF0,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

////1���ⲿ��֤ װ������Կ//  ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��
////   							39����Կ��ʶ��F0��ʹ��Ȩ��F0������Ȩ��AA������״̬��88�������������FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF����Կ��
//// 
////2��װ��PIN ������Կ// 80D401000D   80D401000D37F0F0FF551122334455667788   ������� 55  ��Կ 1122334455667788
//unsigned char static  MF_PIN_Unblock[] ={0x37,0xF0,0xF0,0xFF,0x55,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

////3��װ��PIN��Կ// 80D40100083AF0EF4455123456  ����״̬ 44  ������� 55   pin  123456 
//unsigned char static  MF_PINKey[] ={0x3A,0xF0,0xEF,0x44,0x55,0x12,0x34,0x56 };

////4��װ��ά����Կ// ��װ��Կ       80D4 0100 0D 38F0F0FF550102030405060708    ������� 55  ��Կ0102030405060708
//unsigned char static  MF_AMK[] ={0x38,0xF0,0xF0,0xFF,0x55,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

 

//��MF�½���0015�Ĺ�����Ϣ�ļ�//
unsigned char static  MF_0015[]={0x28,0x00,0x0e,0xF0,0xF0,0xFF,0xFF};

//��MF�½���0016���ն���Ϣ�ļ�//
unsigned char static  MF_0016[]={0x28,0x00,0x06,0xF0,0xF0,0xFF,0xFF};


//��MF�½���0015�Ĺ�����Ϣ�ļ�//
unsigned char static  MF_0015_flag[]={0x00,0x15};

//��MF�½���0016���ն���Ϣ�ļ�//
unsigned char static  MF_0016_flag[]={0x00,0x16};





//-------------------����DATA���ݣ�����DFĿ¼�µ������ļ�����Կ-----------------------------------------------------//

//{0x3F,0x01,0x8F,0x95,0xF0,0xFF,0xFF}; 38036FF0F095FFFFA00000000386980701
//unsigned char static  MF_3F00[] ={0x38,0xFF,0xFF,0xF0,0xF0,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
//80E0 3F 01 0D 380520F0F095FFFF 4444463031
unsigned char static  DF_3F01[] ={0x38,0x05,0x6F,0xF0,0xF0,0x95,0xFF,0xFF,0x44,0x44,0x46,0x30,0x31}; //����DF�ļ�,��PBOC�ļ�,��ʶ��Ϊ3F01//
//{0x3f,0x00,0x50,0x01,0xF0,0xFF,0xFF};3F 01 8F 95 F0 FF FF
unsigned char static  DF_0000[] ={0x3F,0x01,0x8F ,0x95 ,0xF0 ,0xFF ,0xFF};             //����DF�µ�EF��Կ�ļ�,��ʶ��Ϊ0000//

 
//����ڲ���Կ
//    ����ָ�80D4010015 34 F0 02 00 01 34343434343434343434343434343434
//	
//1�� װ���ڲ� ��Կ   װ������Կ//34����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
unsigned char static 	DF_Internal_Key[]={0x34,0xF0,0x02,0x00,0x01,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34};

//2���ļ���·������Կ���������·��Կ��ǰ���Ǹ����������ļ���ͬ�� ����ָ�80D401001536F002FF3336363636363636363636363636363636
    //ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��36����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������36363636363636363636363636363636����Կ��
unsigned char static 	DF_Pl_Key[]={0x36,0xF0,0x02,0xFF,0x77,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36};
//3����Կ��·������Կ
unsigned char static 	DF_P_MAC2_Key[]={0x37,0xF0,0x02,0xFF,0x77,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//3����ӿ��������Կ    ����ָ�80D401001537F002FF3337373737373737373737373737373737    ָ��ظ���9000����ӳɹ���
 //   ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��37����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������37373737373737373737373737373737����Կ��
unsigned char static 	DF_PinUn_Key[]={0x37,0xF0,0x02,0xFF,0x33,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//4��    ��ӿ�����װ��Կ     ����ָ�80D401001538F002FF3338383838383838383838383838383838     ָ��ظ���9000����ӳɹ���
//    ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��38����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������38383838383838383838383838383838����Կ��
unsigned char static 	DF_PinRe_Key[]={0x38,0xF0,0x02,0xFF,0x33,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38};//37,0xF0,0x02,0xFF,0x33,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37};

//5��װ��PIN��Կ// 80D40100083AF0EF4455123456  ����״̬ 44  ������� 55   pin  123456 
unsigned char static  DF_PINKey[] ={0x3A,0xF0,0xEF,0x44,0x55,0x12,0x34,0x56 };

//6��    �ⲿ��֤��Կ     ����ָ�80D401001539F002443339393939393939393939393939393939    ָ��ظ���9000����ӳɹ���
//    ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��39����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��44������״̬��33�������������39393939393939393939393939393939����Կ��
unsigned char static  DF_ExKey[] ={0x39,0xF0,0x02,0x44,0x77,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39,0x39};

//7�� ����޸�͸֧�޶���Կ ����ָ�80 D4 01 01 15 3C F00201003C013C013C013C013C013C013C013C01	 
unsigned char static 	DF_ODKey[]={0x3C ,0xF0,0x02,0x01,0x00,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01,0x3C,0x01};

//8��װ��������Կ//	 80 D4 01 01 15 3E F00200013E013E013E013E013E013E013E013E01                    
unsigned char static 	DF_DPKey[]={0x3E ,0xF0,0x02,0x00,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01,0x3E,0x01};

//9������Ȧ����Կ //      ����ָ�80 D4 01 01 15 3F F0 02 00 01 3F013F013F013F013F013F013F013F01         				
unsigned char static 	Credit_For_Load_KEY[]={0x3F,0xF0,0x02,0x00,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01,0x3F,0x01};
//10������Ȧ����Կ //80 D4 01 01 15 3D F0 02 01003D013D013D013D013D013D013D013D01
unsigned char static 	DCredit_For_Load_KEY[]={0x3D,0xF0,0x02,0x01,0x00,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01,0x3D,0x01};
 
//11��des������Կ // 30����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
unsigned char static 	DF_DES_E_KEY[]={0x30,0xF0,0x02,0x00,0x01,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};

//12��des������Կ // 31����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
unsigned char static 	DF_DES_D_KEY[]={0x31,0xF0,0x02,0x00,0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31};
 
//13��desmac��Կ // 32����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
unsigned char static 	DF_DES_MAC_KEY[]={0x32,0xF0,0x02,0x00,0x01,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32};
 
		

 //  ����15���ļ����������ļ���    ����ָ�80 E0 00 15 07 A8001EF0F0FFFF    ָ��ظ���9000�������ɹ���
//    ָ��˵����80��CLA��E0��INS��0015��P1 P2 �ļ���ʶ��07��Lc��A8������MAC 28(�������ļ���λ��1)�C>A8��001E���ļ��ռ䣩F0����Ȩ�ޣ�F0������Ȩ�ޣ�FF��Ĭ��FF��FF��Ĭ��FF��
//ע��28�C>A8 28=00101000 ��λ��1����10101000=A8������+MACУ�飩
unsigned char static  DF_0015[]={0x28,0x00,0x1E,0xF0,0xF0,0xFF,0xFF};

//    ����17���ļ����������ļ���    ����ָ�80E0001707 2805DCF0F0FFFF    ָ��ظ���9000�������ɹ���
//ָ��˵����80��CLA��E0��INS��0017��P1 P2 �ļ���ʶ��07��Lc��28���������ļ���05DC���ļ��ռ䣩F0����Ȩ�ޣ�F0�����Ȩ�ޣ�FF��Ĭ��FF��FF��Ĭ��FF��
unsigned char static  DF_0016[]={0x28,0x00,0x27,0xF0,0xF0,0xFF,0xFF};

//    ����17���ļ����������ļ���    ����ָ�80E0001707 2805DCF0F0FFFF    ָ��ظ���9000�������ɹ���
//ָ��˵����80��CLA��E0��INS��0017��P1 P2 �ļ���ʶ��07��Lc��28���������ļ���05DC���ļ��ռ䣩F0����Ȩ�ޣ�F0�����Ȩ�ޣ�FF��Ĭ��FF��FF��Ĭ��FF��
unsigned char static  DF_0017[]={0x28,0x00,0x20,0xF0,0xF0,0xFF,0xFF};

 //����18���ļ���ѭ���ļ���    ������ϸ�ļ� ����ָ�80E0001807 2E0A17F0EFFFFF
//80��CLA��E0��INS��0018��P1 P2 �ļ���ʶ��07��Lc��2E��ѭ���ļ���0A17���ļ��ռ䣩F0����Ȩ�ޣ�EF������Ȩ�ޣ�FF��Ĭ��FF��FF��Ĭ��FF��
unsigned char static  DF_0018[]={0x2E,0x0A,0x17,0xF0,0xEF,0xFF,0xFF};

//����Ǯ���ļ�(���Ӵ���)  ����ָ�80 E0 00 01 07 2F0208F100FF18    ָ��ظ���9000�������ɹ���
//    ָ��˵����80��CLA��E0��INS��0001��P1 P2 �ļ���ʶ��07��Lc��2F��PBOC ED/EP��0208��Ĭ��0208��F1��ʹ��Ȩ��00������00��FF��Ĭ��FF��18��������ϸ�ļ��̱�ʶ��
unsigned char static  DF_0001[]={0x2F,0x02,0x08,0xF1,0x00,0xFF,0x18};

//����Ǯ���ļ�(����Ǯ��)     ����ָ�80 E0 00 02 07 2F 02 08 F0 00 FF 18    ָ��ظ���9000�������ɹ���
//ָ��˵����80��CLA��E0��INS��0002��P1 P2 �ļ���ʶ��07��Lc��2F��PBOC ED/EP��0208��Ĭ��0208��F0��ʹ��Ȩ��00������00��FF��Ĭ��FF��18��������ϸ�ļ��̱�ʶ��
unsigned char static  DF_0002[]={0x2F,0x02,0x08,0xF0,0x00,0xFF,0x18};

 

	
unsigned char static  PosNum[]={'L','L',0x30,0x31,0x32,0x33};   //�ն˻���ţ�����������ɷ��������ж��塣		
unsigned char static  MFflag[]={0x3F,0x00};						//MF���ļ���ʶ//
//unsigned char static  MFPkey[]={0x12,0x34,0x56,0x78}; 			//PIN����//

unsigned char static  PublicFile_3F01[]={0x02,0xA0,0x00,0x00,0x00,0x03,0x00,0x00,0x01,0x20,0x16,0x08,0x15,0x00,0x00,0x00,0x01,0x20,0x16,0x10,0x01,0x20,0x18,0x09,0x31};//ȫ����Կ�������Ĺ�����Ϣ�ļ�
unsigned char static  Pos_Transaction_Num[]={0x00,0x00,0x00,0x00,}; //�ն˻�����Ľ������

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
	       //st=PSAM_CreateFile( MF_3F00, sizeof(MF_3F00), Type_MF, Start, revbuff );	//����MF
		  
//		   st=PSAM_Select_File(File_3F00, 2, 0,revbuff, &gRepLen);// 0x28 );	//ѡ��SAM����PBOC��3F00�ļ�// 28
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
//			  UART_Send_Str("���ļ��Ƿ��Ѿ� �� �� ?���ȸ�ʽ��SAM���ٲ���");
//			  while(1);
//		   }
//			UART_Send_Str("\r\nopen 3f 00 ok\r\n");
			#if 1
		   st=PSAM_CreateFile( MF_3F00, sizeof(MF_3F00), 0x3f, 0x00, revbuff );
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
			  UART_Send_Str("���ļ��Ƿ��Ѿ� �� �� ?���ȸ�ʽ��SAM���ٲ���");
			   st=PSAM_Select_File(File_3F00, 2, 0,revbuff, &gRepLen);// 0x28 );	//ѡ��SAM����PBOC��3F00�ļ�// 28
				if(st!=SAM_OK)
				{
					UART_Send_Str("�������:");
					UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
					UART_Send_Str("���ļ��Ƿ��Ѿ� �� �� ?���ȸ�ʽ��SAM���ٲ���");
					while(1);
				}	 
			  //while(1);
		   }
		   __nop();
		   UART_Send_Str("MF�ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
			#endif
		   //------------------------------------------------------------------------------------------------------
		   #if 1
		   st=PSAM_CreateFile( MF_0000, sizeof(MF_0000), Type_EF, Start, revbuff );	//����MF�µ�EF��Կ�ļ�
		   if(st!=SAM_OK)
		   {	  
			  UART_Send_Str("����MF�µ�EF��Կ�ļ� �������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			   while(1);
			  continue;
		   }
		   __nop();
		   UART_Send_Str("MF�µ���Կ�ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
		  #endif
			//0��-----------------Write Key ����ı���ķ�ʽһ -----------------------------------------------------------
	 	   st=PSAMSetKey(MF_CCK_Key,sizeof(MF_CCK_Key), 0x01,0X30, revbuff );   //װ��MF�µ� 0x30����Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue; 	  
	
		   } 
//		 
		   #if 1
		   //1��------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_CMK_Key,sizeof(MF_CMK_Key), 0x01,0x35, revbuff );   //װ��MF�µ�ά����Կ 0x36
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue; 	  
	
		   }
		   __nop();
		   UART_Send_Str("ά����Կ װ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬		  
			

		   //2��------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_PlKey,sizeof(MF_PlKey), 0x01,0x36, revbuff );   //װ��MF�µ� ��·������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("��·������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬


		   //3��------------------------------------------------------------------------------------------------------
		   st=PSAMSetKey(MF_ExKey,sizeof(MF_ExKey), 0x01,0X39, revbuff );   //װ��MF���ⲿ��֤��Կ 0X39
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�ⲿ��֤��Կ �������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("�ⲿ��֤��Կ װ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
 		  #endif
		  

		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( MF_0015, sizeof(MF_0015), 0x00, 0x15, revbuff );	//����MF��0015�Ĺ�����Ϣ�ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			   while(1);
			  continue;	  
	
		   }
		   __nop();
		   UART_Send_Str("0015������Ϣ�ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
		 
		   
		  
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
//		    st=PSAM_Get_challenge(4,revbuff);		       //��SAM����ȡһ�������//    
//		    if(st!=SAM_OK)								 
//		    {
//			    UART_Send_Str("SAM����ȡ��������:");
//				UART_Put_Num(revbuff,2);   //��ʾ�������
//				UART_Send_Enter();			   					            
//			    continue;		
//		    }
// 			memcpy(psam_Challenge_Num, revbuff, 4);     //��ʱ���������
//			memset(&psam_Challenge_Num[4],0, 4);     // 
//			#endif
		   memset(SendData, 0x00, 16);        //��0
		   //memcpy(SendData, &SAM_ATR[6], 10); //PSAM�����к�
		   SendData[9]=0x01;
			SendData[10]=0x01;  		  //PSAM���汾��
		   SendData[11]=0x06; 			      //��Կ������
		   SendData[12]=0x55;  		          //�Զ���FCI����
		   SendData[13]=0x66; 			      //�Զ���FCI����
//		    PSAM_MAC1(0x15 , 14, SendData, &MF_PlKey[5],psam_Challenge_Num,revbuff,tt);
//			printf("mac�������");
//			UART_Put_Num(revbuff,tt[0]);   //��ʾ�������
//			UART_Send_Enter();	
		 
			//FM1208_Update_Binary(0x15,revbuff, tt[0] ));    //ִ�аѼ���·����������д��0015�С�
			 st=PSAM_UpdataBinary(SendData, 14, 0x15,revbuff );
			//st=PSAM_UpdataBinary(revbuff,tt[0],0x15,Rec_Buf );// 0x80|file
			if(st!=SAM_OK)
			{
				UART_Send_Str(" \r\n����:\r\n");
				//UART_Put_Num(Rec_Buf,2);   //��ʾ�������
				while(1);
			}
			UART_Send_Str("0015��Ϣд��ɹ�!");
			UART_Send_Enter();
 
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( MF_0016, sizeof(MF_0016), 0x00, 0x16, revbuff);	//����MF�½���MF_0016�ն���Ϣ�ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
   			  while(1);

			  continue;	  
	
		   }  
		   #if 0
		    st=PSAM_Get_challenge(4,revbuff);		       //��SAM����ȡһ�������//    
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM����ȡ��������:");
				UART_Put_Num(revbuff,2);   //��ʾ�������
				UART_Send_Enter();			   					            
			    continue;		
		    }
 			memcpy(psam_Challenge_Num, revbuff, 4);     //��ʱ���������
			memset(&psam_Challenge_Num[4],0, 4);     // 
		 	 
		    PSAM_MAC1(0x16 , 6, PosNum, &MF_PlKey[5],psam_Challenge_Num,revbuff,tt);
			printf("mac�������\r\n");
			UART_Put_Num(revbuff,tt[0]);   //��ʾ�������
			UART_Send_Enter();	
			#endif
//			st=PSAM_UpdataBinary(PosNum,6,0x16,Rec_Buf );
//			    //ִ�аѼ���·����������д��0016�С�
//			//st=PSAM_UpdataBinary(revbuff,tt[0],0x16,Rec_Buf );// 0x80|file
//			if(st!=SAM_OK)
//			{
//				UART_Send_Str(" \r\n write 0016 ����:\r\n");
//				//UART_Put_Num(Rec_Buf,2);   //��ʾ�������
//				while(1);
//			} 
		   
		    st=PSAM_UpdataBinary(PosNum, 6, 0x16,revbuff );
	 	   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			   while(1);
			  continue;
		
		   }
		   UART_Send_Str("�ɹ�д��0016�ն˻����:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
		  // while(1);
//		   
//		   UART_Send_Str("�ն˻���Ϣ�ļ������ɹ�:");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

		   //------------------------------------------------------------------------------------------------------
	
//		   memset(SendData, 0x00, 16);        //��0
//		   memcpy(SendData, &SAM_ATR[6], 10); //PSAM�����к�
//		   SendData[10]=SAM_ATR[4];  		  //PSAM���汾��
//		   SendData[11]=0x06; 			      //��Կ������
//		   SendData[12]=0x55;  		          //�Զ���FCI����
//		   SendData[13]=0x66; 			      //�Զ���FCI����
//			  

//		   st=PSAM_Select_File(MF_0015_flag, 2, 0,revbuff,&gRepLen);
//		
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//   			  while(1);

//			  continue;
//		
//		   }
//		   UART_Send_Str(" 0015�Ĺ�����Ϣ�򿪳ɹ�:");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
//		   st=PSAM_UpdataBinary(SendData, 14, 0x15,revbuff );
//	 	   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//			   while(1);
//			  continue;
//		
//		   }
//		   UART_Send_Str("��15�ļ��ɹ�");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
//    	           st=PSAM_Select_File(MF_0016_flag, 2, 0,revbuff,&gRepLen);
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//			  continue;
//		
//		   }
//		   UART_Send_Str("��16�ļ��ɹ�");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

//		   //------------------------------------------------------------------------------------------------------


//		
 
		#if 1
		   //------------------------------------------------------------------------------------------------------
   		   st=PSAM_CreateFile( DF_3F01, sizeof(DF_3F01), 0x3f, 0x01, revbuff);	//����MF�µ�DF�ļ�Ϊ3F01
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
  			  while(1);

			   continue;	  
	
		   }
		   UART_Send_Str("3F01�ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
	
			
		 // PSAM_Select_File(unsigned char *pDataIn, unsigned char Lc, unsigned char type,unsigned char* pDataOut, unsigned int *Le );
			st=PSAM_Select_File(File_3F01, 2, 0,revbuff,&gRepLen);
		    if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  while(1);
			   continue;	  
	
		   }
		   UART_Send_Str("3F01�ļ��򿪳ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
		   //------------------------------------------------------------------------------------------------------
   		   st=PSAM_CreateFile( DF_0000, sizeof(DF_0000), 0x00, 0x00, revbuff);	////����DF�µ�EF��Կ�ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ���Կ�ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
		 
//1�� װ���ڲ� ��Կ   װ������Կ//34����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01�������������DF_Internal_Key[]={0x34,
		    st=PSAMSetKey(DF_Internal_Key,sizeof(DF_Internal_Key), 0x01,0x34,revbuff );   //װ��DF�µ�������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�����Կ01װ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////2��·������Կ���������·��Կ��ǰ���Ǹ����������ļ���ͬ�� ����ָ�80D401001536F002FF3336363636363636363636363636363636
//   //ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��36����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������36363636363636363636363636363636����Կ��
// 	DF_Pl_Key[]={0x36,
		   st=PSAMSetKey(DF_Pl_Key,sizeof(DF_Pl_Key), 0x01,0x36,revbuff );   //װ��DF�µ���·������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ���·������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////3����ӿ��������Կ    ����ָ�80D401001537F002FF3337373737373737373737373737373737    ָ��ظ���9000����ӳɹ���
// //   ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��37����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������37373737373737373737373737373737����Կ��
	      st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x37,revbuff );   //װ��DF�µĿ��������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µĿ��������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////4��    ��ӿ�����װ��Կ     ����ָ�80D401001538F002FF3338383838383838383838383838383838     ָ��ظ���9000����ӳɹ���
////    ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��38����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��FF��Ĭ��FF��33�������������38383838383838383838383838383838����Կ��
		    st=PSAMSetKey(DF_PinRe_Key,sizeof(DF_PinRe_Key), 0x01,0x38,revbuff );   //װ��DF�µĿ�����װ��Կ  
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µĿ�����װ��Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////5��װ��PIN��Կ// 80D40100083AF0EF4455123456  ����״̬ 44  ������� 55   pin  123456 
//  DF_PINKey[] ={0x3A,
			st=PSAMSetKey(DF_PINKey,sizeof(DF_PINKey), 0x01,0x3A,revbuff );   //װ��DF�µĿ���PIN��Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�PIN��Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////6��    �ⲿ��֤��Կ     ����ָ�80D401001539F002443339393939393939393939393939393939    ָ��ظ���9000����ӳɹ���
////    ָ��˵����80��CLA��D4��INS��01��P1��00��P2��15��Lc��39����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��44������״̬��33�������������39393939393939393939393939393939����Կ��
//  DF_ExKey[] ={0x39,
    st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x39,revbuff );   //װ��DF�µ��ⲿ��֤��Կ  
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ���·������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////7�� ����޸�͸֧�޶���Կ ����ָ�80 D4 01 01 15 3C F00201003C013C013C013C013C013C013C013C01	 
// 	DF_ODKey[]={0x3C
    st=PSAMSetKey(DF_PinUn_Key,sizeof(DF_PinUn_Key), 0x01,0x3C,revbuff );   //װ��DF�µĿ��������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ���·������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////8��װ��������Կ//	 80 D4 01 01 15 3E F00200013E013E013E013E013E013E013E013E01                    
// 	DF_DPKey[]={0x3E 
    st=PSAMSetKey(DF_DPKey,sizeof(DF_DPKey), 0x01,0x3E,revbuff );   //װ��DF�µĿ���������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////9������Ȧ����Կ //      ����ָ�80 D4 01 01 15 3F F0 02 00 01 3F013F013F013F013F013F013F013F01         				
// 	Credit_For_Load_KEY[]={0x3F,
		       st=PSAMSetKey(Credit_For_Load_KEY,sizeof(Credit_For_Load_KEY), 0x01,0x3F,revbuff );   //װ��DF�µ�Ȧ����Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�Ȧ����Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

////10������Ȧ����Կ 	DCredit_For_Load_KEY[]={0x3D,
// 
		      st=PSAMSetKey(DCredit_For_Load_KEY,sizeof(DCredit_For_Load_KEY), 0x01,0x3D,revbuff );   //װ��DF�µ�Ȧ����Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�Ȧ����Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬


//		  //11��des������Կ // 30����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
//unsigned char static 	DF_DES_E_KEY[]={0x30,0xF0,0x02,0x00,0x01,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
		   	st=PSAMSetKey(DF_DES_E_KEY,sizeof(DF_DES_E_KEY), 0x01,0x30,revbuff );   //װ��DF�µ�des������Կ 
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�des������Կ װ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬


////12��des������Կ // 31����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
//unsigned char static 	DF_DES_D_KEY[]={0x31,0xF0,0x02,0x00,0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31};
	      st=PSAMSetKey(DF_DES_D_KEY,sizeof(DF_DES_D_KEY), 0x01,0x31,revbuff );   //װ��DF�µ�des������Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�des������Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
 
////13��desmac��Կ // 32����Կ��ʶ��F0��ʹ��Ȩ��02������Ȩ��00����Կ�汾��01���㷨��ʶ��
	//unsigned char static 	DF_DES_MAC_KEY[]={0x32,0xF0,0x02,0x00,0x01,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32};
	      st=PSAMSetKey(DF_DES_MAC_KEY,sizeof(DF_DES_MAC_KEY), 0x01,0x32,revbuff );   //װ��DF�µ�desmac��Կ
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF�µ�desmac��Կװ�سɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
 
 
			 
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( DF_0015, sizeof(DF_0015), 0x00, 0x15, revbuff );	//����DF��0015 ����Ӧ�û��������ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("0015����Ӧ�û��������ļ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬


			st=PSAM_CreateFile( DF_0016,sizeof(DF_0016), 0x00, 0x16, revbuff );	//����DF��0016 �ֿ��˻��������ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("0016�ֿ��˻��������ļ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬


			st=PSAM_CreateFile( DF_0017,sizeof(DF_0017), 0x00, 0x17, revbuff );	//����DF��0017�ֿ��˻��������ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("DF��0017 ���������ļ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_CreateFile( DF_0018, sizeof(DF_0018), 0x00, 0x18, revbuff);	//����DF�½���0018�ն˽�������ļ�
		   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;	  
	
		   }
		   UART_Send_Str("0018�ն˽�������ļ������ɹ�:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬
			#endif //while(1);
		   
		//   st=PSAM_Select_File(File_3F01, 2, 0,revbuff, &gRepLen);// 0x28 );
		//   st=PSAM_Select_File(File_0017, 2, 0,revbuff, &gRepLen);
		   //------------------------------------------------------------------------------------------------------
		   st=PSAM_UpdataBinary(PublicFile_3F01, sizeof(PublicFile_3F01), 0x17,revbuff );
	 	   if(st!=SAM_OK)
		   {
			  UART_Send_Str("�������:");
			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
			  continue;
		
		   }		    
		   UART_Send_Str("�ɹ�д��ȫ����Կ�������Ĺ�����Ϣ����:");
		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

		//   st=PSAM_Select_File(File_0018, 2, 0,revbuff, &gRepLen);
		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_UpdataBinary(Pos_Transaction_Num, sizeof(Pos_Transaction_Num), 0x18,revbuff );  
//	 	   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//			  continue;
//		
//		   }
//		    
//		   UART_Send_Str("�ɹ���ʼ���ն˽������:");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

			

//		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_CreateFile( DF_3F01, 2, Type_DF, End, revbuff );	//��������DF
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//			  continue;
//		
//		   }
//		   
//		   UART_Send_Str("DF�ļ���������:");
//		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬



//		   //------------------------------------------------------------------------------------------------------
//		   st=PSAM_CreateFile( MFflag, sizeof(MFflag), Type_MF, End, revbuff );	//��������MF
//		   if(st!=SAM_OK)
//		   {
//			  UART_Send_Str("�������:");
//			  UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬	  
//			  continue;
//		
//		   }
//		   
 		   UART_Send_Str("��������:");
 		   UART_Put_Num(revbuff,2);	//����2���ֽڵ�״ֵ̬

		 
			psam_delay_10ms(200);

			while(1);
	   }

}
#endif
 