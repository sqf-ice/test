#include "nfc.h"
#include "delay.h"
#include "string.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include <stdio.h>
#include "pn532_dev.h"
#include "smartcard.h" 
#include "FM1208.H"
#include "des.h"
#include <string.h>
#include "bsp_serial.h"
 
extern SC_APDU_Resp_t gAPDU_repbuf;
extern uint16_t gAPDU_rxLen;
extern SC_APDU_Cmd_t gAPDU_Send;

#if 0
/////�Զ���ֿ��˻�����Ϣ����////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char static CardTpye=0x00;	//�����ͱ�ʶ��00  ����ֻ��һ�����룬����00��ʾ�й����У�01��ʾ�������еȵȣ�
unsigned char static StaffType=0x00;  //����ְ����ʶ	��ֻ��һ�����룬����00��ʾ����û���01��ʾ��ͨ�û��ȵ�

char name[32] = {"������\0"};	//�ֿ�������
unsigned char static DocumentNum[]={0x31 ,0x31 ,0x30 ,0x31 ,0x30 ,0x32 ,0x39 ,0x38 ,0x31 ,0x32 ,0x31 ,0x38 ,0x30 ,0x30 ,0x31 ,0x30};//�ֿ��˵�֤������	  
unsigned char static DocumentTpye=0x05;//�ֿ��˵�֤������	����05��ʾ���֤��06��ʾ���յȵ�  ,05�ǲ��Ǵ������֤����֪��,Ҫȥ�������е���,���֤���Ĵ�����ʲô

#define	 NameLen     strlen (name)	 //���������ĳ���

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//-------------------����Ϊ��Ҫ�õ�ָ���DATA���ݣ�����MFĿ¼�µ������ļ�����Կ-----------------------------------------------------//
//
//Ϊͳһ��������Ķ����룬���������е��ļ�������Ӹ�_XXXX,��ʾ�����ļ��ı�ʶ�ţ�//														
//���磺 FM1208_MF_Key_0000  :��ʾFM1208 CPU�� ��MF �� KEY�ļ�������Կ�ļ�����ʶ��Ϊ0000
//


//����Կ�ļ�
unsigned char static FM1208_MF_Key_0000[] ={0x3F, 0x00, 0x50, 0x01, 0xF0, 0xFF, 0xFF};	

//1��39װ��16�ֽڵ��ⲿ��֤��Կ//	
unsigned char static  FM1208_MF_ExKey[] ={0x39,0xF0,0xF0,0xAA,0x88,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	
									
//2��36װ��8�ֽڵ���·������Կ	   
unsigned char static  FM1208_MF_LineCKey[]={0x36,0xF0,0xF0,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//��������¼�ļ���������¼�ļ�����Ϊ���ļ�������Ϣ��file control information�� 
unsigned char static  FM1208_MF_RECORD[]={0x2A,0x02,0x13,0xF0,0x00,0xFF,0xFF};

//װ�� ��¼�ļ���Ŀ¼�� ��PBOC �� ASC �������
unsigned char static  FM1208_MF_RECORD_DATA[]={0x61,0x11,0x4F,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x50,0x04,0x50,0x42,0x4F,0x43};





//-------------------����Ϊ��Ҫ�õ�ָ���DATA���ݣ�����DFĿ¼�µ������ļ�����Կ-----------------------------------------------------//
//
//Ϊͳһ��������Ķ����룬���������е��ļ�������Ӹ�_XXXX,��ʾ�����ļ��ı�ʶ�ţ�//
//���磺 FM1208_DF_3F01  :��ʾFM1208 CPU�� ��DF ��	��ʶ��Ϊ3F01
//


//����DF�ļ�,��ʶ��Ϊ3F01//
unsigned char static  FM1208_DF_3F01[] ={0x38,0x03,0x6F,0xF0,0xF0,0x95,0xFF,0xFF,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01};

//����DF�µ���Կ�ļ�
unsigned char static FM1208_DF_Key_0000[] ={0x3F,0x01,0x8F,0x95,0xF0,0xFF,0xFF};	//����Կ�ļ�

//1��װ��TAC��Կ
unsigned char static  FM1208_DF_TACKey[] ={0x34,0xF0,0x02,0x00,0x01,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34};

//2����·������Կ��MAC�����õ���   ����SAM����MAC��Կһ����ɢ�ó���
unsigned char static  FM1208_DF_LineCKey[] ={0x36,0xF0,0x02,0xFF,0x33,0x7C,0x51,0xF0,0x79,0x10,0xD8,0x03,0x8D,0x20,0x97,0xE5,0x8F,0xDA,0x79,0x3F,0x1E};

//3���ⲿ��֤��Կ  ����SAM����ɢ���ã�
unsigned char static  FM1208_DF_ExKey[] ={0x39,0xF0,0x02,0x44,0xFF,0x5F,0xD6,0x76,0x0C,0xDF,0x6D,0xD3,0x93,0xFF,0xDD,0x6C,0x5C,0xCB,0x65,0x85,0xBF};


//4��������Կ  ��PSAM����������Կ���ӷ�ɢ��ȡ��
unsigned char static  FM1208_DF_DPKey[] ={0x3E,0xF0,0x02,0x00,0x01,0x3E,0x5E,0x67,0xF2,0xFF,0x23,0x09,0x29,0x9F,0x84,0x20,0xF4,0x3D,0xE1,0x39,0x2B};

//5��Ȧ����Կ    (��PSAM����MAC��������Կ��Ȼ���û������к���Ϊ���ӷ�ɢ��������ΪȦ����Կ��������ÿ���û�������Կ��ͬ)
unsigned char static  FM1208_DF_CZKey[] ={0x3F,0xF0,0x02,0x00,0x01,0x4A,0x17,0xCA,0x82,0x98,0xD5,0x28,0x65,0x14,0x0A,0xBE,0xAA,0x10,0x8A,0x4E,0x00 }; 
	  
//6������PIN��Կ	  
unsigned char static  FM1208_DF_PIN[]={0x3A,0xF0,0xEF,0x01,0x33,0x12,0x34,0x56,0xFF,0xFF,0xFF,0xFF,0xFF}; 

//7���ڲ���Կ	  ����SAM����ɢ���ã�
unsigned char static  FM1208_DF_INKey[]={0x30,0xF0,0xF0,0xFF,0x33,0x1E,0xE2,0xFC,0x1F,0x6C,0x47,0xD2,0x55,0xBF,0x4F,0xE9,0x41,0xF6,0x02,0xFB,0x29};




//����0015 ����Ӧ�û��������ļ�	 �Ǵ���·�����Ķ������ļ�  
unsigned char static  FM1208_DF_0015[] ={0xA8,0x00,0x1E,0xF0,0xF0,0xFF,0xFF};

//����Ӧ�û��������ļ�����  ע��: ������һ���û������кţ�00062016102000000001��ȡ��8���ֽڣ����Ǹ�ÿ���û�������ʱ����Ϊ��ͬ
unsigned char static  FM1208_DF_0015_DATA[] ={0x11,0x11,0x22,0x22,0x33,0x33,0x00,0x06,0x03,0x01,0x00,0x06,0x20,0x16,0x10,0x20,0x00,0x00,0x00,0x01,0x20,0x16,0x10,0x20,0x20,0x19,0x12,0x31,0x55,0x66};//


//����0016 ED/EP�ֿ��˻�����Ϣ���ݵ��ļ����磬������֤������Ϣ �Ǵ���·�����Ķ������ļ�   
unsigned char static  FM1208_DF_0016[] ={0xA8,0x00,0x27,0xF0,0xF0,0xFF,0xFF};

											
//��0018 ED/EPӦ�õĽ�����ϸѭ����¼�ļ��� ����ֻҪ������ļ����ü��ɣ������ǽ��׳ɹ���COSϵͳ���Զ�д�룬���������Լ�д��ġ�
unsigned char static  FM1208_DF_0018[] ={0x2E,0x0A,0x17,0xF1,0xEF,0xFF,0xFF};

//��0001 ED���Ӵ����ļ�
unsigned char static  FM1208_DF_0001[] ={0x2F,0x02,0x08,0xF1,0x00,0xFF,0x18};

//��0002 EP����Ǯ���ļ�		
unsigned char static  FM1208_DF_0002[] ={0x2F,0x02,0x08,0xF0,0x00,0xFF,0x18};



//////////////////�ļ���ʶ�Ķ���/////////////////////////////////////////////////////////////////////////////////
//
//Ϊͳһ��������Ķ����룬���������е��ļ�������Ӹ�_Flag,��ʾ���Ǹö�Ӧ�ļ��ı�ʶ//

//unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF�ļ���ʶ3F00
unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF�µ���Կ�ļ���ʶ0000
unsigned char static ExKey_Flag       = {0x00};           //MF�µ��ⲿ��Կ��ʶ00
unsigned char static LineKey_Flag     = {0x01}; 		    //MF�µ���·������Կ��ʶ01
unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF�µĶ�����¼�ļ���ʶ0001
unsigned char static RECORD_Flag      = {0x08}; 		    //MF�µĶ�����¼��ʶ08

unsigned char static FM1208_DF_3F01_Flag[] ={0x3f,0x01};	//DF�ļ���ʶ3F01
unsigned char static KEY_File_0000_Flag[2] ={0x00,0x00};  //DF�µ���Կ�ļ���ʶ0000				   
unsigned char static FM1208_DF_TACKey_Flag={0x00};		//DF�µ�TAC��Կ��ʶ00
unsigned char static FM1208_DF_LineCKey_Flag={0x00};		//DF�µ���·������Կ��ʶ00
unsigned char static FM1208_DF_ExKey_Flag  = {0x00};      //DF�µ��ⲿ��Կ��ʶ00
unsigned char static FM1208_DF_Inital_Key_Flag  = {0x00}; //DF�µ��ڲ���Կ��ʶ00



unsigned char static FM1208_DF_DPKey_Flag={0x00};			//DF�µ�������Կ��ʶ00
unsigned char static FM1208_DF_CZKey_Flag={0x00};			//DF�µ�Ȧ����Կ��ʶ00
unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF�µĿ���PIN��Կ��ʶ00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//����Ӧ�û��������ļ���ʶ0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//�ֿ��˻�����Ϣ���ݵ��ļ���ʶ0016
unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //������ϸ�ļ���ʶ0018
unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//���Ӵ����ļ���ʶ0001
unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//����Ǯ���ļ���ʶ0002




unsigned char  Set_CpuCard(void)
{
	 char st;
	 unsigned int  sta;
	 unsigned char tt[2];
	 unsigned char  Challenge[8]={0,0,0,0,0,0,0,0};
	 unsigned char  stdata[3];



	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000, sizeof(FM1208_MF_Key_0000), FM1208_MF_Key_0000 )); //����Կ�ļ�
		if(st)	
		return 1;		//���󷵻�	

		UART_Send_Str("��ʶΪ0000����Կ�ļ������ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);

		st=SendUartStatus(FM1208_Load_Key(ExKey_Flag, sizeof(FM1208_MF_ExKey), FM1208_MF_ExKey));	//װ���ⲿ��֤��Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("�ⲿ��֤��Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;

							

 		st=SendUartStatus(FM1208_Load_Key(LineKey_Flag, sizeof(FM1208_MF_LineCKey), FM1208_MF_LineCKey));	//װ����·������Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("��·������Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&LineKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;



 		st=SendUartStatus(FM1208_CreateFile(RECORD_File, sizeof(FM1208_MF_RECORD), FM1208_MF_RECORD));	//����������¼�ļ�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("����������¼�ļ��ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(AppendRecord(RECORD_Flag, sizeof(FM1208_MF_RECORD_DATA), FM1208_MF_RECORD_DATA));	 //д������¼�ļ�����
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("д������¼�ļ����ݳɹ�!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;

	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_3F01_Flag, sizeof(FM1208_DF_3F01), FM1208_DF_3F01 )); //��DF�ļ�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("��DF�ļ�3F01�ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_SelectFile(FM1208_DF_3F01_Flag,DataBuf,tt)); //��CPU��3F01�ļ�//
		if(st)	
		return 1;		//���󷵻�
		
		UART_Send_Str("3F01�ļ��򿪳ɹ�:"); 
		UART_Put_Num(&DataBuf[2],tt[0]);	 //����������ʾ���ص���?
	    UART_Send_Enter();
		//delay(5);;
		

	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000_Flag, sizeof(FM1208_DF_Key_0000), FM1208_DF_Key_0000 )); //��DF�µ���Կ�ļ�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�±�ʶΪ0000����Կ�ļ������ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_TACKey_Flag, sizeof(FM1208_DF_TACKey), FM1208_DF_TACKey));	//װ��TAC��Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("TAC��Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_TACKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;
	

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_LineCKey_Flag, sizeof(FM1208_DF_LineCKey), FM1208_DF_LineCKey));	//װ����·������Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µ���·������Կװ�سɹ�,��ʶΪ");
		UART_Put_Num(&FM1208_DF_LineCKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_ExKey_Flag, sizeof(FM1208_DF_ExKey), FM1208_DF_ExKey));	//װ���ⲿ��֤��Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µ��ⲿ��֤��Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_DPKey_Flag, sizeof(FM1208_DF_DPKey), FM1208_DF_DPKey));	//װ��������Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µ�������Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_DPKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_CZKey_Flag, sizeof(FM1208_DF_CZKey), FM1208_DF_CZKey));	//װ��Ȧ����Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µ�Ȧ����Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_CZKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_PIN_Flag, sizeof(FM1208_DF_PIN), FM1208_DF_PIN));	//װ�ؿ���PIN��Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µĿ���PINԿװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_PIN_Flag,1);
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_Inital_Key_Flag, sizeof(FM1208_DF_INKey), FM1208_DF_INKey));	//װ���ڲ���Կ
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�µ��ڲ�Կװ�سɹ�,��ʶΪ:");
		UART_Put_Num(&FM1208_DF_Inital_Key_Flag,1);
		UART_Send_Enter();
		//delay(5);;
		sta=FM1208_CreateFile(FM1208_DF_0015_Flag, sizeof(FM1208_DF_0015), FM1208_DF_0015 );  //��DF�µ�0015�ļ�
		if(sta!=0x9000)	//������Ϊ01ʱ����ʾ��ʱ
		{
			 tt[0]=sta>>8;
		     if(tt[0]==0x01)	  //����01ʱ����Ҫ�ȴ�ʱ������
			 {
				while(1)
				{
					WTX_Cmd(stdata,tt);
					if(stdata[2]!=0x01)
					{
						sta=0x9000;
						break;
					}
					else
					{
						__nop();
						//delay_ms(5);
					}
				}
			 }
		}

		st=SendUartStatus(sta);
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�±�ʶΪ0015�ļ������ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(4,Challenge);	 //ȡ�����	
		MAC1(FM1208_DF_0015_Flag[1], sizeof(FM1208_DF_0015_DATA), FM1208_DF_0015_DATA, &FM1208_DF_LineCKey[5],Challenge,DataBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0015_Flag[1],DataBuf, tt[0] ));    //ִ�аѼ���·����������д��0015�С�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("0015����Ӧ�û�������д��ɹ�!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;


	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0016_Flag, sizeof(FM1208_DF_0016), FM1208_DF_0016 )); //��DF�µ�0016�ļ�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("DF�±�ʶΪ0016�ļ������ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(4,Challenge);	 				 //ȡ�����		
		DataBuf[0]=CardTpye;							 //�����ͱ�ʶ
		DataBuf[1]=StaffType;							 //����ְ����ʶ
		memcpy(&DataBuf[2], name, NameLen+1);			 //�ֿ�������
		memset(&DataBuf[NameLen+2], 0x00, 20-NameLen);	 //�������油0
		memcpy(&DataBuf[22], DocumentNum, 16);			 //�ֿ��˵�֤������
		DataBuf[38]=DocumentTpye;						 //�ֿ��˵�֤������


		MAC1(FM1208_DF_0016_Flag[1], 0x27, DataBuf, &FM1208_DF_LineCKey[5],Challenge,g_cReceBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, tt[0] ));    //ִ�аѼ���·����������д��0016�С�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("0016�û���������Ϣд��ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;	



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0018_Flag, sizeof(FM1208_DF_0018), FM1208_DF_0018 )); //��DF�µ�0018�ļ�
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("��DF�µ�0018ѭ�����׼�¼�ļ��ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0001_Flag, sizeof(FM1208_DF_0001), FM1208_DF_0001 )); //��DF�µĵ��Ӵ����ļ�ED
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("��DF�µĵ��Ӵ���ED�ļ��ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0002_Flag, sizeof(FM1208_DF_0002), FM1208_DF_0002 )); //��DF�µĵ���Ǯ���ļ�EP
		if(st)	
		return 1;		//���󷵻�
		UART_Send_Str("��DF�µĵ���Ǯ��EP�ļ��ɹ�!");
		UART_Send_Enter();
		UART_Send_Enter();


		return 0;


}

#endif

//*************************************************************************
// ������	��pro_reset
// ����		��ת��APDU�����ʽCPU����λ����
// ���		�� ��
// ����		�� DataOut ��������ݣ�	rlen ������ݵĳ���
// ����		���ɹ����� _OK = 0
//*************************************************************************

char pro_reset(unsigned char *DataOut, unsigned char *rlen)
{

//		char status;
//	unsigned char  st[2];
//	unsigned int   sst=0;
	uint8_t sendbuf[10];
 //	uint16_t i = 0;	 
	/***********  MfComData.MfCommand = PCD_TRANSCEIVE;		 
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = 0xE0;
    MfComData.MfData[1] = 0x60;
      ************/
	sendbuf[0] = 0xE0;
	sendbuf[1] = 0x60;
 
	if(CardTransmit(sendbuf,2, gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		* rlen = gAPDU_rxLen;
		return   0; 
	 
	}
	else
		return 0xFF; 
	
 
	//PcdSetTmo(4);


  
 
	 
}

//*************************************************************************
//  
// ������	��FM1208_SelectFile
// ����		��ѡ���ļ�
// ���		��pDataIn : �ļ���ʶ������DF ����
// ����		��pDataOut �������   Out_Len��������ݳ���
// ����		��ִ�гɹ�sst=9000
//*************************************************************************
 
unsigned int FM1208_SelectFile(unsigned char* pDataIn, unsigned char* pDataOut,unsigned char * Out_Len)
{
//	char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	gAPDU_Send.Header.CLA 	= 0;     // CLA
	gAPDU_Send.Header.INS 	= 0xA4;   // INS-Select File 
	gAPDU_Send.Header.P1	= 0;		// P1
	gAPDU_Send.Header.P2 	= 0;		// P2
	gAPDU_Send.Body.LC 		= 0x02;		// LC
	gAPDU_Send.Body.Data[0] = pDataIn[0]; 	// File
	gAPDU_Send.Body.Data[1] = pDataIn[1]; 	// File
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,7,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
	 
		* Out_Len = gAPDU_rxLen-2;
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF;   
	
	return 0xff;
 
	  
}

//*************************************************************************
// zirun
// ������	��WTX_Cmd
// ����		���˺�����Ҫ�����ǣ�����CPU�����ļ���ɾ���ļ�ʱ����Ҫ�ṩ�ȴ�ʱ�䣬
//            ��ִ��CPU��ָ���01ʱ���������01���ͻظ�CPU����ֱ���ȵ����ٷ���01��ִ�гɹ�
// ���		��pDataIn : Ҫִ�е�ָ��					
// ����		��pDataOut�����ִ�к�ķ�������   Out_Len��������ݳ���
// ����		��0
//*************************************************************************

char  WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len)
{
  
//	char status;
//	unsigned char  st[2];
//	unsigned int   sst=0;
	unsigned char send_dat[8];
//	uint16_t len;
 
	 send_dat[0] = 0xFA;
     send_dat[1] = 0x00;
	 send_dat[2] = 0X01;
	
	if(CardTransmit( send_dat,3,gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{
		* Out_Len = gAPDU_rxLen-2;
		memcpy(pDataOut, gAPDU_repbuf.Data,*Out_Len);
		return 0; 
	}
	else
		return 0xFF;   
	
 
  							
 
	
 }
//*************************************************************************
// zirun 
// ������	��FM1208_GetChallenge
// ����		����ȡ�����
// ���		��Len  4/8/16		  		
// ����		�� pRan: 4���ֽڵ������
// ����		��ִ�гɹ�sst=0X9000
//*************************************************************************
unsigned int FM1208_GetChallenge(unsigned char Len,unsigned char* pRan)
{

//     char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 
	uint16_t i = 0;	
	gAPDU_Send.Header.CLA 	= 0;
	gAPDU_Send.Header.INS 	= 0x84;
	gAPDU_Send.Header.P1	= 0;
	gAPDU_Send.Header.P2 	= 0;
	gAPDU_Send.Body.LC 		= Len;
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}	 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			memcpy(pRan, gAPDU_repbuf.Data, Len);
			return sst;
		} 
	}
	else
		return 0xFF;
	return 0xFF;

}

//*************************************************************************
//  
// ������	��FM1208_CreateFile
// ����		�������ļ�
// ���		��fileid :�������ļ���ʶ
//            Len ���������ݵĳ���       
//			  pDataIn�������ļ����ĵ�����
//
// ����		����
// ����		��ִ�гɹ�sst=0X9000
//*************************************************************************
 
unsigned int  FM1208_CreateFile(unsigned char *fileid, unsigned char Len, unsigned char *pDataIn )
{
//	char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****
	*******/
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0xE0;   // INS-Select File 
	gAPDU_Send.Header.P1	= fileid[0];		// P1
	gAPDU_Send.Header.P2 	= fileid[1];		// P2
	gAPDU_Send.Body.LC 		= Len;		// LC
	memcpy(gAPDU_Send.Body.Data,pDataIn,Len);
	
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5+Len,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF;   
 return 0xFF;
 
}

//*************************************************************************
//  
// ������	��FM1208_Load_Key
// ����		��װ����Կ
// ���		��Keysign:��Կ��ʶ
//			  Len ���������ݵĳ��� 		
//			  pDataIn��װ����Կ�ļ����ĵ�����
//					
//					
// ����		����
// ����		��ִ�гɹ�sst=0X9000
//*************************************************************************

unsigned int FM1208_Load_Key(unsigned char Keysign, unsigned char Len, unsigned char* pDataIn)
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****
	*******/
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0xD4;   // INS-Write Key 
	gAPDU_Send.Header.P1	=  0x01;		// P1
	gAPDU_Send.Header.P2 	=  Keysign;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Len;		// LC
	memcpy(gAPDU_Send.Body.Data,pDataIn,Len);
	
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5+Len,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
//		if( gAPDU_repbuf.SW1 == 0x67)
//		{
//			//���ȴ���
//		}
//		if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
//		{
//			//��֧�ִ˹��� 
//		}
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 	
	return 0xFF;
}
 


//*************************************************************************
//  
// ������	��AppendRecord
// ����		��д��¼�ļ�
// ���		��fileid ��¼�ļ��Ķ��ı�ʶ
//			  Len ���������ݵĳ��� 	
//			  pDataIn��д���¼�ļ��ı�������
//				
//				
// ����		����
// ����		��ִ�гɹ�sst=0X9000
//*************************************************************************

unsigned int AppendRecord(unsigned char fileid, unsigned char Len, unsigned char* pDataIn)
{
	
	
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****   
	*******/
	gAPDU_Send.Header.CLA 	= 0x00;     // CLA
	gAPDU_Send.Header.INS 	= 0xE2;   // INS-Write Key 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  fileid;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Len;		// LC
	memcpy(gAPDU_Send.Body.Data,pDataIn,Len);
	
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5+Len,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}	 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 	
	return 0xFF;

}

//*************************************************************************
//  
// ������	��FM1208_Update_Binary
// ����		��д�������ļ����Ǵ���·����
// ���		��File_Flag: �������ļ��Ķ��ı�ʶ
//            pDataIn :��Ҫд��Ķ���������
//            Len:���������ݵĳ���
//		  		
// ����		����
// ����		��ִ�гɹ�sst=0X9000
//*************************************************************************

unsigned int FM1208_Update_Binary(unsigned char File_Flag,unsigned char *pDataIn, unsigned char Len )
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****   
	 
	*******/
	gAPDU_Send.Header.CLA 	= 0x04;     // CLA
	gAPDU_Send.Header.INS 	= 0xD6;   // INS-Write Key 
	gAPDU_Send.Header.P1	=  0x80|File_Flag;		// P1
	gAPDU_Send.Header.P2 	=  0x00;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Len;		// LC
	memcpy(gAPDU_Send.Body.Data,pDataIn,Len);
	
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5+Len,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
//		if( gAPDU_repbuf.SW1 == 0x67)
//		{
//			//���ȴ���
//		}
//		if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
//		{
//			//��֧�ִ˹��� 
//		}
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 	
	return 0xFF;
  
}
 
//*************************************************************************
//  
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
void MAC1(unsigned char File_Flag, unsigned char Len, unsigned char *pDataIn, unsigned char *LineCKey, unsigned char *Challenge,unsigned char *pDataOut,unsigned char *Le)
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
//************************************************
// ������	��FM1208_External_Authenticate
// ����		���ⲿ��֤
// ���		��pDataIn ����֤������ 
//			  Key_Flag : ��Կ��ʶ
//			 
//
// ����		��pDataOut ������ݣ����ܺ�Ľ��
// ����		���ɹ�����sst=9000
//*************************************************************************
					 
unsigned int FM1208_External_Authenticate(unsigned char Key_Flag,unsigned char *pDataIn,unsigned char *pDataOut)
{
	
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****
 
	*******/
	gAPDU_Send.Header.CLA 	= 0x00;     // CLA
	gAPDU_Send.Header.INS 	= 0x82;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  Key_Flag;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x08;		// LC
	memcpy(gAPDU_Send.Body.Data,pDataIn,8);
	
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;
}
// ������	��FM1208_Format
// ����		����ʽ��FM1208�����ѿ�����MF�µ�ȫ������ɾ����
// ���		����    
// 			  
//
//
// ����		����
// ����		���ɹ�����sst=9000
//*************************************************************************

unsigned  int  FM1208_Format(void)
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	
	/*****
   
	*******/
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x0E;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  0x00;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x00;		// LC
	 
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;

}

// ������	��FM1208_ReadNomey
// ����		���������
// ���		��type : 01��Ϊ���Ӵ��ۡ� 02Ϊ����Ǯ�� 
//			 
//			  
//
// ����		��pDataOut ������ݣ��ɹ��򷵻�4���ֽڵĿ����
// ����		���ɹ�����sst=9000
//*************************************************************************

unsigned int FM1208_ReadNomey(unsigned char type ,unsigned char *pDataOut)
{
	
		unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
 
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x5C;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  type;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x04;		// LC
	 
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;
	
}

// ������	��FM1208_Inital_Authenticate
// ����		���ڲ���֤
// ���		��pDataIn ����֤������ 
//			  Key_Flag : ��Կ��ʶ
//			  Lc:��֤���ݵĳ���
//
// ����		��pDataOut ������ݣ����ܺ�Ľ��
// ����		���ɹ�����sst=9000
//*************************************************************************
 
unsigned int FM1208_Inital_Authenticate(unsigned char Key_Flag, unsigned char Lc, unsigned char *pDataIn,unsigned char *pDataOut)
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	 
	gAPDU_Send.Header.CLA 	= 0x00;     // CLA
	gAPDU_Send.Header.INS 	= 0x88;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  Key_Flag;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Lc;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,Lc);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;
	
      
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
unsigned int FM1208_Verify_Pin(unsigned char File_Flag,unsigned char Len, unsigned char *pDataIn)
{
	
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	   
	gAPDU_Send.Header.CLA 	= 0x00;     // CLA
	gAPDU_Send.Header.INS 	= 0x20;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  File_Flag;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Len;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,Len);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			__nop();
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;
	
  
} 
// ������	��FM1208_Read_Binary
// ����		�����������ļ� 
// ���		��Len:Ҫ�������ݳ���             
//			  File_Flag :�������ļ��Ķ��ı�ʶ
//			
//
// ����		��pDataOut ������������
// ����		��sst=9000 �ɹ�
//*************************************************************************

unsigned int FM1208_Read_Binary(unsigned char File_Flag,unsigned char *pDataOut, unsigned char Len )
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	   
 
 
	gAPDU_Send.Header.CLA 	= 0x00;     // CLA
	gAPDU_Send.Header.INS 	= 0xB0;   // INS- 
	gAPDU_Send.Header.P1	=  0x80|File_Flag;		// P1
	gAPDU_Send.Header.P2 	=  0x00;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= Len;		// LC
	// memcpy(gAPDU_Send.Body.Data,pDataIn,0x0B);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			__nop();
			return sst;
		} 
	}
	else
		return 0xFF; 
	
 return 0xFF; 

}

// ������	��FM1208_InitalizeForLoad
// ����		��Ȧ���ʼ��
// ���		��pDataIn ���ļ���ʶ������DF ����  
//			  type : 01�����ڵ��Ӵ���   02�����ڵ���Ǯ��
//			  
//
// ����		��pDataOut ������ݣ�δ��ֵǰ�ľ���α�������
// ����		���ɹ�����sst=9000
//*************************************************************************

unsigned int FM1208_InitalizeForLoad(unsigned char type, unsigned char *pDataIn,unsigned char *pDataOut)
{
	unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	   
 
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x50;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  type;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x0B;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,0x0B);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			__nop();
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF;    
}


//*************************************************************************
 
// ������	��FM1208_CreditForLoad
// ����		��Ȧ������
// ���		��pDataIn �����������: ���׵�����ʱ�� ��MAC2 
//			 
//			  
//
// ����		��pDataOut ������ݣ��ɹ��򷵻�4���ֽڵ�TAC����
// ����		���ɹ�����sst=9000
//*************************************************************************

unsigned int FM1208_CreditForLoad(unsigned char *pDataIn,unsigned char *pDataOut)
{
		unsigned char  st[2];
	unsigned int   sst=0;
 //	uint16_t i = 0;	   
 
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x52;   // INS- 
	gAPDU_Send.Header.P1	=  0x00;		// P1
	gAPDU_Send.Header.P2 	=  0x00;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x0B;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,0x0B);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			//ֻȡҪ�õ����ݳ���		 
			//memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
			__nop();
			return sst;
		} 
	}
	else
		return 0xFF; 
	
	return 0xFF; 

}
// ������	��FM1208_Debit_For_Capp_Purchase
// ����		������ִ����������
// ���		��pDataIn ���ն˽�����š���������ʱ�䡢MAC1�� 
//			  
//			
//
// ����		��pDataOut ���ɹ��󷵻أ�������֤TAC�롢MAC1�� 
// ����		��sst=9000 �ɹ�
//*************************************************************************			   
unsigned int FM1208_Debit_For_Capp_Purchase (unsigned char *pDataIn,unsigned char *pDataOut,unsigned char *Len)
{
		unsigned char  st[2];
	unsigned int   sst=0;
 
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x54;   // INS- 
	gAPDU_Send.Header.P1	=  0x01;		// P1
	gAPDU_Send.Header.P2 	=  0x00;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x0F;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,0x0F);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
		if(st[0] == 0x61)
			sst = 0x9000;
		return sst;
//		if(sst==0x9000)
//		{
//			//ֻȡҪ�õ����ݳ���		 
//			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
//			__nop();
//			return sst;
//		} 
	}
	else
		return 0xFF; 
	
	return 0xFF; 
}

 
//*************************************************************************
//  
// ������	��FM1208_Initalize_For_Purchase
// ����		������ִ�����ѳ�ʼ��
// ���		��type: 01 ���Ӵ��� �� 02 ���Ӵ���             
//			  pDataIn ��������Կ��ʶ�����׽��ն˻����
//			
//
// ����		��pDataOut ���ɹ��󷵻أ����Ӵ��ۻ����Ǯ������� ���û���������š�α������� 
// ����		��sst=9000 �ɹ�
//*************************************************************************						   
unsigned int FM1208_Initalize_For_Purchase(unsigned char type, unsigned char *pDataIn,unsigned char *pDataOut, unsigned char *Len)
{
		unsigned char  st[2];
	unsigned int   sst=0;
 
 
	
	gAPDU_Send.Header.CLA 	= 0x80;     // CLA
	gAPDU_Send.Header.INS 	= 0x50;   // INS- 
	gAPDU_Send.Header.P1	=  0x01;		// P1
	gAPDU_Send.Header.P2 	=  type;		// P2 p2-key flag
	gAPDU_Send.Body.LC 		= 0x0B;		// LC
	 memcpy(gAPDU_Send.Body.Data,pDataIn,0x0B);
	if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,(5+gAPDU_Send.Body.LC),(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{ 
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			 
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
 
		 
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW2; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
		return sst;
//		if(sst==0x9000)
//		{
//			//ֻȡҪ�õ����ݳ���		 
//			memcpy(pDataOut, gAPDU_repbuf.Data, gAPDU_rxLen-2);
//			__nop();
//			return sst;
//		} 
	}
	else
		return 0xFF; 
	
	return 0xFF; 
}
