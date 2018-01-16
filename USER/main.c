 #include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include <string.h>
#include "t8029.h"
#include "nfc.h"
#include "des.h"
#include "encryption.h"
#include "pn532_dev.h"
#include "psam1.h"
#include "smartcard.h" 
#include "bsp_led.h"  
#include "fm1208.h"  
#include "iso14443a.h"
#include "bsp_serial.h"
#include "msg_process.h"
#define EPOS_DEBUG 0
//#define SYSTICK		 720000		//10MS
#define SYSTICK		 1440000		//5MS
#define STM32_UNIQUEID_ADDR   0x1FFFF7E8


 
#define OPEN_SAM_FILE3F00    		0x00 
#define READ_SAM_POSNUM		 		0x01
#define OPEN_SAM_FILE3F01    		0x02 
#define OPEN_CPU_FILE3F01    		0x03
#define READ_CPU_CARDNUM	 		0x04
//
#define DES_INIT_SAM_CPU_EXTH    	0x05
#define DES_CALC_SAM_CPU_EXTH    	0x06
#define EXTH_CPU_AUTH			 	0x07
#define DES_RAND_CPU_INTH		 	0x08
#define DES_INIT_SAM_CPU_INTH    	0x09
#define DES_CALC_SAM_CPU_INTH    	0x0A
 
#define READ_CPU_FILE0016_INF   	0x0B
#define INIT_PURCH_CPU 			    0x0C

#define INIT_PURCH_PSAM		 		0x0D
  
#define DB_PURCH_CPU 				0x0E
#define CHECK_MAC2_PSAM				0x0F
#define PURCHASE_OK 			    0x10

//gCpuReadState
unsigned char  g_cReceBuf[MAXRLEN]; 
unsigned char  DataBuf[MAXRLEN];
unsigned int   gRepLen;
unsigned char  msg_sendbuf[MAXRLEN];
 //-------------------����DATA���ݣ�����MFĿ¼�µ������ļ�����Կ-----------------------------------------------------//

                              

unsigned char  CardNum[8]; //����û������к�

unsigned char static  S_KEY[16]={0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; //��Ա���б�ʶ���ӣ�ǰ8���ֽڣ�,�Ե���б�ʶ����(��8���ֽ�)




//////////////////�ļ���ʶ�Ķ���/////////////////////////////////////////////////////////////////////////////////
//
//Ϊͳһ��������Ķ����룬���������е��ļ�������Ӹ�_Flag,��ʾ���Ǹö�Ӧ�ļ��ı�ʶ//
//
//unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF�ļ���ʶ3F00
//unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF�µ���Կ�ļ���ʶ0000
//unsigned char static ExKey_Flag       = {0x00};           //MF�µ��ⲿ��Կ��ʶ00
//unsigned char static LineKey_Flag     = {0x01}; 		    //MF�µ���·������Կ��ʶ01
//unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF�µĶ�����¼�ļ���ʶ0001
//unsigned char static RECORD_Flag      = {0x08}; 		    //MF�µĶ�����¼��ʶ08

//unsigned char static CZKey_Flag={0x01};	  //Ȧ����Կ��ʶΪ01
unsigned char static DPKey_Flag={0x00};	  //������Կ��ʶΪ00
unsigned char static DF_ExKey_Flag  = {0x00};      //DF�µ��ⲿ��Կ��ʶ00
unsigned char static DF_Inital_Key_Flag  = {0x00}; //DF�µ��ڲ���Կ��ʶ00


//unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF�µĿ���PIN��Կ��ʶ00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//����Ӧ�û��������ļ���ʶ0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//�ֿ��˻�����Ϣ���ݵ��ļ���ʶ0016
//unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //������ϸ�ļ���ʶ0018
//unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//���Ӵ����ļ���ʶ0001
//unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//����Ǯ���ļ���ʶ0002




////////////////////////////////////////////////////////////////////////
//�ļ���ʶ

 
unsigned char static File_0016_s={0x16};	        //�ն˻����ļ���ʶ

unsigned char  static     DateTime[]={0x20,0x17,0x10,0x26,0x11,0x32,0x45};
unsigned char             gRecDateTime[7];
unsigned char      Temporary_Key[8];  //��Ź�����Կ
unsigned char      Old_Pay[4];        //��ž�Ǯ������δ����ǰ����
unsigned char      Challenge_Num[8];  //�����
unsigned char      Serial_Num[2];     //�������
unsigned char      MAC1_Num[4];       //MAC1ֵ

unsigned char      NameBuf[20];	 //��ʱ�������
unsigned char      NameLen;		 //��������

unsigned char  gPurchase_Pay[4]={0x00,0x00,0x00,0x01}; //�����Ľ�� ��λ��ǰ��16���Ƽ���ģ���������1Ԫ
uint32_t gPurchaseNum;
unsigned char  gPurchaseFlag =0;
unsigned int PAY;



unsigned char  POSNum[6]; 								  //�ն˻����	��SAM���ж�ȡ
 
static void RCC_Configuration(void)
{
	//�ο���ַ��http://blog.csdn.net/iamlvshijie/article/details/9249545
	ErrorStatus HSEStartUpStatus; 
	RCC_DeInit();     //����RCC�Ĵ�����������ΪĬ��ֵ
	RCC_HSEConfig (RCC_HSE_ON);//���ⲿ����ʱ�Ӿ���HSE��
	HSEStartUpStatus = RCC_WaitForHSEStartUp();//�ȴ��ⲿʱ�Ӿ����ȶ�����
	if(HSEStartUpStatus == SUCCESS)//SUCCESS��HSE�����ȶ��Ҿ���
	{
		//FLASH_PrefetchBufferCmd(ENABLE);          //����FLASH��Ԥȡ����
		//FLASH_SetLatency(FLASH_Latency_2);      //FLASH�ӳ���������
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//����AHBʱ�Ӳ���Ƶ
		RCC_PCLK2Config(RCC_HCLK_Div1); //����APB2ʱ�Ӳ���Ƶ
		RCC_PCLK1Config(RCC_HCLK_Div2); //����APB1ʱ�Ӷ���Ƶ ��Ӧ�Ķ�ʱ����ʱ��Ϊ2��Ƶ
//	FLASH_SetLatency(FLASH_Latency_2);//����FLASH�洢����ʱ��������FLASH_Latency_2 2��ʱ����
//	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//ѡ��FLASHԤȡָ�����ģʽ��Ԥȡָ����ʹ��
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);//����PLL   
		RCC_PLLCmd(ENABLE); //PLLʱ��ʹ��
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource()!=0x08);//�ȴ�������
	}
	else
	{
		__nop();
	}
//	RCC_LSICmd(ENABLE);
//  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
// 
	//��������˿�ʱ��
	//ʹ��PA~PE�˿�ʱ��IN1~IN32
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 
//	//��������1��2��3ʱ��
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//ʹ��USART1ʱ��
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2��USART3ʱ��
//	
	/* Configure HCLK clock as SysTick clock source. */
	//SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
 
}
 
/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */
uint32_t gMainTicks;
uint32_t gBsp_uart_ticks;
SC_APDU_Resp_t gAPDU_repbuf;
uint16_t gAPDU_rxLen;
SC_APDU_Cmd_t gAPDU_Send;
static void main_delay_10ms(uint16_t n10ms)
{
	uint32_t Delaytick;

	Delaytick = BspGetSysTicks();
	while(!BspGetDlyTicks(Delaytick,n10ms));

}
 //���������ĳ��ȣ�����ʱ��������
static void NameToStr(unsigned char *s)	  
{
	 char i;
	 NameLen=strlen((char*)s); 
	 for(i=0;i<NameLen;i++)
	 	NameBuf[i]=s[i];

}
 
 
 
 
unsigned char File_3F00[]={0x3f,0x00};
unsigned char File_3F01[]={0x3f,0x01};
unsigned char File_0015[]={0x00,0x15};
unsigned char File_0016[]={0x00,0x16};
unsigned char File_0017[]={0x00,0x17};
unsigned char File_0018[]={0x00,0x18};

unsigned char Challenge_Num[8];
unsigned char cpu_ex_key[]={0x5F,0xD6,0x76,0x0C,0xDF,0x6D,0xD3,0x93,0xFF,0xDD,0x6C,0x5C,0xCB,0x65,0x85,0xBF};
void iso_or_config(void);
uint8_t g_bsp_uart_send_buf[256];
uint8_t last_UID[4];
uint8_t gDeviceID[2];
/**********************************************************
* �������� ---> ��ȡCPU 96λΨһID
* ��ڲ��� --->  
* ������ֵ ---> 
* ����˵�� ---> none
**********************************************************/
void GetSTM32ID(uint32_t *Device_Serial,uint8_t *wID)
{
	//uint32_t Device_Serial0,Device_Serial1,Device_Serial2;
	*Device_Serial++ = *(uint32_t*)(STM32_UNIQUEID_ADDR);      //12 Bytes Serial Number
	*Device_Serial++ = *(uint32_t*)(STM32_UNIQUEID_ADDR + 4);
	*Device_Serial++ = *(uint32_t*)(STM32_UNIQUEID_ADDR + 8);
	CRC16((u8*)Device_Serial,12,wID);
}
 int main(void)
{	
	char status = 0;
	T_MSG_RESP *pt_msg_resp =(T_MSG_RESP*) g_tMsg.Data; 
	uint16_t t_msg_len;
 	unsigned char tt[2];
 	unsigned int st,i,j;	
 	unsigned char stdata[3];
	static unsigned char gCpuReadState =0;
	//SystemInit();
	RCC_Configuration();
	SysTick_Config(SYSTICK);
	bsp_led_config();
	bsp_serial_config();
	TDA8029_IO_Config( );
    TDA1_Usart3_Config( );
 	PN532_DevConfig();

	/*����PN532	 Wired Card, the host controller can access to the SAM with standard PCD commands (InListPassiveTarget,InDataExchange, ��)*/
 	
	
	gMainTicks = BspGetSysTicks(); 	
	PSAM_Config();
	if(TDA8029_CheckPluggedDevice() == TDA8029_OK)
	{
		//UPLOAD_DEVICE_STATUS
		msg_sendbuf[0] = 0x01;
		msg_sendbuf[1] = 0x00;
		

		//printf("have found device tda8029\r\n");
	}
	else
	{
		msg_sendbuf[0] = 0x01;
		msg_sendbuf[1] = 0xee;
		
	}
	msg_pack(UPLOAD_DEVICE_STATUS,msg_sendbuf,2,&g_tMsg );
	TDA8029_CheckCardIn();
	TDA8029_CardPowerOn();
	if(PN532_SAMConfiguration(0x01,0x17,0) == 1)
	{
		//find out pn532
		msg_sendbuf[0] = 0x02;
		msg_sendbuf[1] = 0x00;
	}
	else
	{
		msg_sendbuf[0] = 0x02;
		msg_sendbuf[1] = 0xee;
		
	}
	msg_pack(UPLOAD_DEVICE_STATUS,msg_sendbuf,2,&g_tMsg );
	GetSTM32ID((uint32_t*)g_cReceBuf,gDeviceID);
	gBsp_uart_ticks = BspGetSysTicks();
	LED1_ON; 
  	while (1)
  	{
		if((uint16_t)(BspGetSysTicks() -gBsp_uart_ticks ) > 100)
		{
			gBsp_uart_ticks = BspGetSysTicks();
			//g_tMsg.
			//LED1_TOGGLE; 
			if(g_tMsg.wMsgType  == 1)
			{
				g_tMsg.wMsgType = 0;
				t_msg_len = pt_msg_resp->header.wLen[0] + (pt_msg_resp->header.wLen[0] << 8);
				UART_Send_Data(g_tMsg.Data,t_msg_len);//UART_Put_Num(g_tMsg.Data,t_msg_len);
				
			}
		}
		 
//		LED1_TOGGLE; 
//		main_delay_10ms(200);
//		LED1_TOGGLE;
 
//	
		#if 1
		if(!nfc_InListPassiveTarget(0x01,0,UID))
		{
			continue; //Ѱ��
		}
		//printf("\r\n start time is %d ",BspGetSysTicks());
		if(!nfc_InSelect(1))
			continue;
		
		if(memcmp(last_UID,UID,4) != 0)
		{
			memcpy(last_UID,UID,4);
			gPurchaseFlag = 1;
			gCpuReadState = 0;
			LED2_TOGGLE;
		}	
		//LED2_TOGGLE;
			switch(gCpuReadState)
			{
				case OPEN_SAM_FILE3F00:
					gRepLen = 0x28 ;			
					st=PSAM_Select_File(File_3F00, 2, 0,g_cReceBuf, &gRepLen);// 0x28 );	//ѡ��SAM����PBOC��3F00�ļ�// 28
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM��3F00�ļ��򿪴���:");
						UART_Put_Num(g_cReceBuf,16);   //��ʾ�������
						UART_Send_Enter();	
						#endif
					   gCpuReadState = 0; break;//continue;
				
					}
					gCpuReadState ++;
				case READ_SAM_POSNUM:
					gRepLen = 6;
					st=PSAM_ReadBinary( File_0016_s,g_cReceBuf,&gRepLen); //  6 );	 //��ȡSAM��������ն˻����//
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM�����ն˻���Ŷ�ȡ����:");
						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
						UART_Send_Enter();
						#endif
   					   gCpuReadState = 0; break;//continue;
				
					}
					#if EPOS_DEBUG
					UART_Send_Str("�ն˻����Ϊ:");						 
					UART_Put_Num(g_cReceBuf,6); UART_Send_Enter();
					#endif
					memcpy(POSNum, g_cReceBuf, 6);
					gCpuReadState ++;
				case OPEN_SAM_FILE3F01:
					st=PSAM_Select_File(File_3F01, 2, 0,g_cReceBuf,&gRepLen); // 0x27 );	//ѡ��SAM����PBOC��3F01�ļ�
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM��PBOC�ļ��򿪴���:");
						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
						UART_Send_Enter();			   
						#endif						
					   gCpuReadState = 0; break;//continue;
				
					}					 
					#if EPOS_DEBUG
					UART_Send_Str("SAM��PBOC�ļ��򿪳ɹ�!");
					UART_Send_Enter();
					UART_Send_Enter();
					#endif
					gCpuReadState ++; //break;
				case OPEN_CPU_FILE3F01:
					st=FM1208_SelectFile(File_3F01,g_cReceBuf,tt);      //��CPU����PBOC��3F01�ļ�//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							UART_Send_Str("�û���PBOC�ļ��򿪴���:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);
							UART_Send_Enter();
						}
						 gCpuReadState = 0; break;//continue;continue; 
					}
					#if EPOS_DEBUG 
						UART_Send_Str(">>�û���PBOC�ļ��򿪳ɹ�<<"); UART_Send_Enter();
					#endif
					gCpuReadState ++;
				case READ_CPU_CARDNUM:					
					st=FM1208_Read_Binary(FM1208_DF_0015_Flag[1],g_cReceBuf, 0x1E );		  //��ȡ0015�ļ�����Ӧ�û�������//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							UART_Send_Str("����Ӧ�û����ļ���ȡʧ��:"); 
							UART_Put_Num(g_cReceBuf,2);
							UART_Send_Enter();
						}
						 gCpuReadState = 0; break;//continue;continue; 
					}
					#if EPOS_DEBUG 
					 UART_Send_Str(">>����Ӧ�û����ļ���ȡ�ɹ�<<"); 
					 UART_Send_Enter();					 UART_Send_Enter();
					#endif
					
     				 memcpy(CardNum, &g_cReceBuf[12], 8);   //�����û������
					#if EPOS_DEBUG 
					 UART_Send_Str("�û������:"); 
					 UART_Put_Num(CardNum,8);				//�û������
					 UART_Send_Enter();
					#endif
					gCpuReadState ++; //					break;
	
				case DES_INIT_SAM_CPU_EXTH:					
//					st=FM1208_GetChallenge(8,g_cReceBuf);			   //���û���ȡһ�������//
//					if(st!=0x9000)
//					{	
//						if(st!=ST_ERR)
//						{
//							#if EPOS_DEBUG 
//							UART_Send_Str("�û�����ȡ��������:"); 
//							UART_Put_Num(g_cReceBuf,2);
//							UART_Send_Enter();
//							#endif
//						}
//						 gCpuReadState = 0; break;//continue;continue; 
//					}
//					memcpy(Challenge_Num, g_cReceBuf, 8);     //��ʱ���������
//					st=Init_for_descrypt(CardNum, 8, 0x27,DF_ExKey_Flag, g_cReceBuf);     //��3DE��ʼ�� 1����ɢ����������Կ    27--��Կ��;,Ϊ������Կ     
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 

//						UART_Send_Str("SAM����3DES��ʼ��ʧ��:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();
//						#endif						
//						 gCpuReadState = 0; break;// 
//					}
					gCpuReadState ++;
				case DES_CALC_SAM_CPU_EXTH:					 
//					st=PSAM_Descrypt(Challenge_Num, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES�㷨 �Ըղ���ʱ�������������м���//
//					if(st!=SAM_OK)
//					{
//						#if EPOS_DEBUG 

//						UART_Send_Str("SAM����3DES�������:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;
//					}
//					#if EPOS_DEBUG 

//				//	 Single_3DES_Encrypt(Challenge_Num,cpu_ex_key,g_cReceBuf);
//					UART_Send_Str("�ⲿ��֤�ļ��ܽ��:"); 
//					UART_Put_Num(g_cReceBuf,8);	UART_Send_Enter();
//					#endif
//					
//					memcpy(DataBuf, g_cReceBuf, 8);	  //��ʱ������ܽ��
					gCpuReadState ++;
				case EXTH_CPU_AUTH:
					
//					st=FM1208_External_Authenticate(DF_ExKey_Flag,DataBuf,g_cReceBuf);  //ִ���ⲿ��֤//
//					if(st!=0x9000)
//					{	
//						if(st!=ST_ERR)
//						{
//							#if EPOS_DEBUG 

//							UART_Send_Str("�ն�POS���Ƿ�,�ⲿ��֤����:"); 
//							UART_Put_Num(g_cReceBuf,2);
//							UART_Send_Enter();
//							#endif
//						}
//						gCpuReadState = 0; break;//continue;while(1);
//						 
//					}
//					#if EPOS_DEBUG  
//					UART_Send_Str("�ⲿ��֤�ɹ�,�ն�POS���Ϸ�!"); 
//					UART_Send_Enter();
//					UART_Send_Enter();
//					#endif
					gCpuReadState ++;
			case DES_RAND_CPU_INTH:
//					st=PSAM_Get_challenge(8,g_cReceBuf);		       //��SAM����ȡһ�������//    
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM����ȡ��������:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}
//					memcpy(Challenge_Num, g_cReceBuf, 8);     //��ʱ���������
//			 		st=FM1208_Inital_Authenticate(DF_Inital_Key_Flag, 8, Challenge_Num,g_cReceBuf); //�û����ڲ���֤,���ܸ������//
//					if(st!=0x9000)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("�ڲ���֤����:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();			   					            
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}
//					memcpy(DataBuf, g_cReceBuf, 8);	  //��ʱ������ܽ��
					gCpuReadState ++;		 
				case DES_INIT_SAM_CPU_INTH:
//					st=Init_for_descrypt(CardNum, 0x8, 0x2C, DF_Inital_Key_Flag, g_cReceBuf);     //��3DE��ʼ�� 1����ɢ����������Կ    2C--��Կ��;,Ϊ������Կ  
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM����3DES��ʼ��ʧ��:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}					
					gCpuReadState ++;
				case DES_CALC_SAM_CPU_INTH:
//					st=PSAM_Descrypt(DataBuf, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES�㷨 �Ըղ���ʱ����ļ����˵����ݽ��н���//
//					if(st!=SAM_OK)
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM����3DES�������:");
//						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;
//					}
//					#if EPOS_DEBUG 

//					UART_Send_Str("�ڲ���֤�Ľ��ܽ��:"); 
//					UART_Put_Num(g_cReceBuf,8);	
//					UART_Send_Enter();
//					#endif
//				
//					for(i=0;i<8;i++)
//					{
//						if(g_cReceBuf[i]!=Challenge_Num[i])		//�����жϿ��ĺϷ���//
//						{
//						  // send to epos
//							
//						  #if EPOS_DEBUG 
//						  UART_Send_Str("�Ƿ���!�����ڱ��ն�ʹ��!");
//						  UART_Send_Enter();
//						  #endif
//						  
//						  gCpuReadState = 0; break;//continue;while(1);continue;				
//						}
//					}
//					#if EPOS_DEBUG 
//						UART_Send_Str("�ڲ���֤�ɹ�,�û����Ϸ�!");
//						UART_Send_Enter();					UART_Send_Enter();
//					#endif
					gCpuReadState ++;
				case READ_CPU_FILE0016_INF:	
 					st=FM1208_Read_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, 0x27 );		  //��ȡ0016�ļ��ֿ��˻�����Ϣ
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("�ֿ��˻�����Ϣ�ļ���ȡʧ��:"); 
							UART_Put_Num(g_cReceBuf,2);
							UART_Send_Enter();
							#endif
						}
						gCpuReadState = 0; break;//continue;while(1);continue;				
 
					}
					#if EPOS_DEBUG 

					UART_Send_Str(">>�ֿ��˻�����Ϣ�ļ���ȡ�ɹ�<<"); 
					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("�����ͱ�ʶ:"); 
					UART_Put_Num(&g_cReceBuf[0],1);
					UART_Send_Enter();

					UART_Send_Str("����ְ����ʶ:"); 
					UART_Put_Num(&g_cReceBuf[1],1);
					UART_Send_Enter();

					UART_Send_Str("�� �� �� ������:"); 
					UART_Send_Str((char*)&g_cReceBuf[2]); 
					NameToStr(&g_cReceBuf[2]);


					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("�� �� �� ��֤������:"); 
					UART_Put_Num(&g_cReceBuf[38],1);
					UART_Send_Enter();

					UART_Send_Str("�� �� �� ��֤������:"); 
					UART_Put_Num(&g_cReceBuf[22],16);
					UART_Send_Enter();
					#endif
					
					DataBuf[0]=DPKey_Flag;	              //1���ֽڵ�������Կ��ʶ					
					memcpy(&DataBuf[1], gPurchase_Pay, 4); //4���ֽڵĽ��׽����ۿ���	
					memcpy(&DataBuf[5], POSNum, 6);	      //6���ֽڵ��ն˻����
					gCpuReadState ++;
		
			
				case INIT_PURCH_CPU:
					st=FM1208_Initalize_For_Purchase(EP, DataBuf,g_cReceBuf,tt); //���ѳ�ʼ������//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("��ʼ�����Ѵ���:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);	  //������9403ʱ����ʱ���һ������û��������������Կ�ı�ʶ�ţ��Ƿ��Ӧ��
							UART_Send_Enter();
							#endif
							if(st==0x9401)
							{
							  //DisNotMoney();	 //������
								msg_sendbuf[0] = 0x94;
								msg_sendbuf[1] = 0x01;
								msg_pack(UPLOAD_SOME_ERROR,msg_sendbuf,2,&g_tMsg );

								
							}
							//main_delay_10ms(2000);
							 
						}

 						  gCpuReadState = 0; break;//continue;while(1);continue;				

					}
					#if EPOS_DEBUG 
					UART_Put_Num(g_cReceBuf,4);
					

					UART_Send_Str(">>���ѳ�ʼ���ɹ�<<"); 
					UART_Send_Enter();
					UART_Send_Enter();
										 
					UART_Send_Str("����Ǯ��δ����ǰ�����:"); 
					UART_Put_Num(g_cReceBuf,4);
					UART_Send_Enter();

					UART_Send_Str("����Ǯ �� �� �����:"); 
					UART_Put_Num(&g_cReceBuf[4],2);
					UART_Send_Enter();

					UART_Send_Str(" �� ��:"); 
					UART_Put_Num(&g_cReceBuf[6],3);
					UART_Send_Enter();

					UART_Send_Str("��Կ�汾��:"); 	
					UART_Put_Num(&g_cReceBuf[9],1);
					UART_Send_Enter();

					UART_Send_Str("�㷨��ʶ:"); 
					UART_Put_Num(&g_cReceBuf[10],1);
					UART_Send_Enter();

					UART_Send_Str("α�����:"); 
					UART_Put_Num(&g_cReceBuf[11],4);
					UART_Send_Enter();
					#endif	
					
//					UART_Send_Str("����Ǯ��δ����ǰ�����:"); 
//					UART_Put_Num(g_cReceBuf,4);
//					UART_Send_Enter();
					memcpy(&msg_sendbuf[8],g_cReceBuf,4);
					memcpy(msg_sendbuf,CardNum,8);
					msg_pack(UPLOAD_REMAINING_MONEY,msg_sendbuf,4+8,&g_tMsg );
					
					PAY=g_cReceBuf[2]<<8;		  
					PAY|=g_cReceBuf[3];					
					memcpy(DataBuf, &g_cReceBuf[11], 4);	  //��α�����
					memcpy(&DataBuf[4], &g_cReceBuf[4], 2);	  //�û����������
					memcpy(&DataBuf[6], gPurchase_Pay, 4);	  //���׽��
					DataBuf[10]=0x06;						  //��������
					memcpy(&DataBuf[11], DateTime, 7);	      //��������ʱ��
					 
					DataBuf[18]=DPKey_Flag;                   //������Կ�汾��ʶ��
					DataBuf[19]=0x00;                         //�㷨��ʶ��3DES
					memcpy(&DataBuf[20], S_KEY, 16);	      //��Ա�����б�ʶ���Ӽ������б�ʶ����
					memcpy(&DataBuf[36], CardNum, 8);	      //�û������	
					gPurchaseNum = (gPurchase_Pay[0] << 24) + (gPurchase_Pay[1] << 16)+(gPurchase_Pay[2] << 8) +gPurchase_Pay[3];
					if((gPurchaseFlag ==1)&& (gPurchaseNum > 0))
					{
						gCpuReadState ++;
					}
					else
						break;
		 
				case INIT_PURCH_PSAM:
				////////////////////////////////////////////////////////////////////			  
					st=Init_SAM_For_Purchase(DataBuf, 0x2C,  g_cReceBuf); //MAC1�����ʼ��// 
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG 
						UART_Send_Str("MAC1�������:");       //������9403ʱ����ʱ���һ�����SAM�������������Կ�İ汾��ʶ�ţ��Ƿ��Ӧ��
						UART_Put_Num(g_cReceBuf,0x02);						UART_Send_Enter();
						#endif
						gCpuReadState = 0; break;//continue;while(1);continue;				 
					}
					#if EPOS_DEBUG 
					UART_Send_Str(">>MAC1�����ʼ���ɹ�<<"); 
					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("�ն˽������:"); 
					UART_Put_Num(g_cReceBuf,0x04);
					UART_Send_Enter();		  
					UART_Send_Str("���ص�MAC1��:"); 
					UART_Put_Num(&g_cReceBuf[4],0x04);		  
					UART_Send_Enter();
					#endif

					memcpy(DataBuf, g_cReceBuf, 4); 		 //���ն˽������	
					memcpy(&DataBuf[4], DateTime, 7); 		 //����������ʱ��
					memcpy(&DataBuf[11], &g_cReceBuf[4], 4); //��MAC1  
					gCpuReadState ++;
				case DB_PURCH_CPU:					
					st=FM1208_Debit_For_Capp_Purchase(DataBuf,g_cReceBuf,tt); //��������//
					if(st!=0x9000)
					{
						
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("�ۿ�ʧ��:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);	 //������9302ʱ����ʱ���һ�������Կ�����Ƿ���ȷ,�����û������к�,��Ա���м����д���,������Կ���Ӵ��ˣ���ɢ���Ĺ�����ԿҲ�Ǵ�ģ����Լ������MACҲ�Ǵ��
							UART_Send_Enter();
							 #endif
							//main_delay_10ms(200);
						}
						gCpuReadState = 0; break;//continue; 
					}
					#if EPOS_DEBUG 
					
					UART_Send_Str("�û������صĽ�����֤TAC:"); 
					UART_Put_Num(g_cReceBuf,4);	
					UART_Send_Enter();	  
					UART_Send_Str("�û������ص�MAC1:"); 
					UART_Put_Num(&g_cReceBuf[4],4);		  
					UART_Send_Enter();
					 #endif
					memcpy(DataBuf, &g_cReceBuf[4], 4); 		 //��MAC2,Ȼ���͸�PSAM������У��
				case CHECK_MAC2_PSAM:
					st=Credit_SAM_For_Purchase(DataBuf, g_cReceBuf);   //MAC2У��//
					if(st!=SAM_OK)
					{
						
						UART_Send_Str("У��MAC2����:");
						UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
						UART_Send_Enter();
						 
						//main_delay_10ms(200);
					  gCpuReadState = 0; break;//continue;while(1);continue; continue; 
					}
					//UART_Send_Str("MAC2У��ۿ�ɹ�!"); 
					//printf("\r\n end time is %d \r\n",BspGetSysTicks());

					gCpuReadState++;
				case PURCHASE_OK:
					gCpuReadState =0;
					gPurchaseFlag = 0;
					memset(gPurchase_Pay,0,4);
					//PURCHASE_SUCCESS
					msg_sendbuf[0] = 'O';
					msg_sendbuf[1] = 'K';
					msg_sendbuf[2] = 0x0D;
					msg_sendbuf[3] = 0x0A;
					msg_pack(UPLOAD_PURCHASE_SUCCESS,msg_sendbuf,4,&g_tMsg );
					break; 
				default: 
					gCpuReadState = 0;
					break;
					
					
					
			}
		
		#endif
	

	 }
  	 
}


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

		#if 0
		if(!nfc_InListPassiveTarget(0x01,0,UID))
			{
				continue; //Ѱ��
			}
			
			gRepLen = 0x28 ;			
		    st=PSAM_Select_File(File_3F00, 2, 0,g_cReceBuf, &gRepLen);// 0x28 );	//ѡ��SAM����PBOC��3F00�ļ�// 28
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM��3F00�ļ��򿪴���:");
				UART_Put_Num(g_cReceBuf,16);   //��ʾ�������
				UART_Send_Enter();			   
			   continue;
		
		    }
			gRepLen = 6;
			st=PSAM_ReadBinary( File_0016_s,g_cReceBuf,&gRepLen); //  6 );	 //��ȡSAM��������ն˻����//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM�����ն˻���Ŷ�ȡ����:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   
			   continue;
		
		    }
			UART_Send_Str("�ն˻����Ϊ:");
			memcpy(POSNum, g_cReceBuf, 6); 	 
			UART_Put_Num(POSNum,6); UART_Send_Enter();

		    st=PSAM_Select_File(File_3F01, 2, 0,g_cReceBuf,&gRepLen); // 0x27 );	//ѡ��SAM����PBOC��3F01�ļ�
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM��PBOC�ļ��򿪴���:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   
			   					   	while(1);		            
		            
			   continue;
		
		    }
			UART_Send_Str("SAM��PBOC�ļ��򿪳ɹ�!");
			UART_Send_Enter();
			UART_Send_Enter();
 			
			st=FM1208_SelectFile(File_3F01,g_cReceBuf,tt);      //��CPU����PBOC��3F01�ļ�//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("�û���PBOC�ļ��򿪴���:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);
					UART_Send_Enter();
				}
							   	while(1);		            

				continue; 
			}
			UART_Send_Str(">>�û���PBOC�ļ��򿪳ɹ�<<"); 
			UART_Send_Enter();
			UART_Send_Enter();
 

			st=FM1208_Read_Binary(FM1208_DF_0015_Flag[1],g_cReceBuf, 0x1E );		  //��ȡ0015�ļ�����Ӧ�û�������//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("����Ӧ�û����ļ���ȡʧ��:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
			   	while(1);		            

				continue; 
			}
			 UART_Send_Str(">>����Ӧ�û����ļ���ȡ�ɹ�<<"); 
			 UART_Send_Enter();
			 UART_Send_Enter();
		    
			 memcpy(CardNum, &g_cReceBuf[12], 8);   //�����û������
			 UART_Send_Str("�û������:"); 
			 UART_Put_Num(CardNum,8);				//�û������
			 UART_Send_Enter();
			  
			//////////////////////////////////////////////////////////////
			//��Ҫ���û����ж��ն˵ĺϷ��ԣ����Ƿ�������α����ն˻�
			
			st=FM1208_GetChallenge(8,g_cReceBuf);			   //���û���ȡһ�������//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("�û�����ȡ��������:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
			   	while(1);		            

				continue; 
			}
			memcpy(Challenge_Num, g_cReceBuf, 8);     //��ʱ���������
			
				st=Init_for_descrypt(CardNum, 8, 0x27,DF_ExKey_Flag, g_cReceBuf);     //��3DE��ʼ�� 1����ɢ����������Կ    27--��Կ��;,Ϊ������Կ     
				if(st!=SAM_OK)								 
				{
					// UART_Send_Str("SAM����3DES��ʼ��ʧ��:");
					UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
					UART_Send_Enter();	
					// while(1);
					//continue;		
				}
	 
		 
			// while(1);
			st=PSAM_Descrypt(Challenge_Num, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES�㷨 �Ըղ���ʱ�������������м���//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM����3DES�������:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();	
			   	while(1);		            
				
			    continue;
		    }
		//	 Single_3DES_Encrypt(Challenge_Num,cpu_ex_key,g_cReceBuf);
			UART_Send_Str("�ⲿ��֤�ļ��ܽ��:"); 
			UART_Put_Num(g_cReceBuf,8);	
			memcpy(DataBuf, g_cReceBuf, 8);	  //��ʱ������ܽ��
			UART_Send_Enter();

			//while(1);
			st=FM1208_External_Authenticate(DF_ExKey_Flag,DataBuf,g_cReceBuf);  //ִ���ⲿ��֤//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("�ն�POS���Ƿ�,�ⲿ��֤����:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
				while(1);
				continue; 
			}

			UART_Send_Str("�ⲿ��֤�ɹ�,�ն�POS���Ϸ�!"); 
			UART_Send_Enter();
			UART_Send_Enter();
			 
			////////////////////////////////////////////////////////////////////
			 
			//////////////////////////////////////////////////////////////
			//��Ҫ�����ն��жϿ��ĺϷ��ԣ�Ҳ����˵�������û����Ƿ������ҵ��ն���ʹ��
		    st=PSAM_Get_challenge(8,g_cReceBuf);		       //��SAM����ȡһ�������//    
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM����ȡ��������:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   					            
			    continue;		
		    }
			memcpy(Challenge_Num, g_cReceBuf, 8);     //��ʱ���������

			
			st=FM1208_Inital_Authenticate(DF_Inital_Key_Flag, 8, Challenge_Num,g_cReceBuf); //�û����ڲ���֤,���ܸ������//
		    if(st!=0x9000)								 
		    {
			    UART_Send_Str("�ڲ���֤����:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   					            
			    continue;		
		    }
			memcpy(DataBuf, g_cReceBuf, 8);	  //��ʱ������ܽ��

			
			st=Init_for_descrypt(CardNum, 0x8, 0x2C, DF_Inital_Key_Flag, g_cReceBuf);     //��3DE��ʼ�� 1����ɢ����������Կ    2C--��Կ��;,Ϊ������Կ  
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM����3DES��ʼ��ʧ��:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   					            
			    continue;		
		    }
	
			st=PSAM_Descrypt(DataBuf, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES�㷨 �Ըղ���ʱ����ļ����˵����ݽ��н���//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM����3DES�������:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();			   					            
			    continue;
		    }

			UART_Send_Str("�ڲ���֤�Ľ��ܽ��:"); 
			UART_Put_Num(g_cReceBuf,8);	
			UART_Send_Enter();


			for(i=0;i<8;i++)
			{
				if(g_cReceBuf[i]!=Challenge_Num[i])		//�����жϿ��ĺϷ���//
				{
				  UART_Send_Str("�Ƿ���!�����ڱ��ն�ʹ��!");
				  UART_Send_Enter();
				  continue;				
				}
			}

		    UART_Send_Str("�ڲ���֤�ɹ�,�û����Ϸ�!");
		    UART_Send_Enter();
			UART_Send_Enter();
			////////////////////////////////////////////////////////////////////			  
			  
			  
			  
			  
			   
			
			st=FM1208_Read_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, 0x27 );		  //��ȡ0016�ļ��ֿ��˻�����Ϣ
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("�ֿ��˻�����Ϣ�ļ���ȡʧ��:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}

				continue; 
			}

			UART_Send_Str(">>�ֿ��˻�����Ϣ�ļ���ȡ�ɹ�<<"); 
			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("�����ͱ�ʶ:"); 
			UART_Put_Num(&g_cReceBuf[0],1);
			UART_Send_Enter();

			UART_Send_Str("����ְ����ʶ:"); 
			UART_Put_Num(&g_cReceBuf[1],1);
			UART_Send_Enter();

			UART_Send_Str("�� �� �� ������:"); 
			UART_Send_Str((char*)&g_cReceBuf[2]); 
			NameToStr(&g_cReceBuf[2]);


			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("�� �� �� ��֤������:"); 
			UART_Put_Num(&g_cReceBuf[38],1);
			UART_Send_Enter();

			UART_Send_Str("�� �� �� ��֤������:"); 
			UART_Put_Num(&g_cReceBuf[22],16);
			UART_Send_Enter();


			
			DataBuf[0]=DPKey_Flag;	              //1���ֽڵ�������Կ��ʶ					
			memcpy(&DataBuf[1], gPurchase_Pay, 4); //4���ֽڵĽ��׽����ۿ���	
			memcpy(&DataBuf[5], POSNum, 6);	      //6���ֽڵ��ն˻����


		    st=FM1208_Initalize_For_Purchase(EP, DataBuf,g_cReceBuf,tt); //���ѳ�ʼ������//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("��ʼ�����Ѵ���:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);	  //������9403ʱ����ʱ���һ������û��������������Կ�ı�ʶ�ţ��Ƿ��Ӧ��
					UART_Send_Enter();

					if(st==0x9401)
					{
					  //DisNotMoney();	 //������
					}
					main_delay_10ms(2000);
					 
				}

				continue; 
			}
			
			UART_Send_Str(">>���ѳ�ʼ���ɹ�<<"); 
			UART_Send_Enter();
			UART_Send_Enter();
								 
			UART_Send_Str("����Ǯ��δ����ǰ�����:"); 
			UART_Put_Num(g_cReceBuf,4);
			UART_Send_Enter();

			UART_Send_Str("����Ǯ �� �� �����:"); 
			UART_Put_Num(&g_cReceBuf[4],2);
			UART_Send_Enter();

			UART_Send_Str(" �� ��:"); 
			UART_Put_Num(&g_cReceBuf[6],3);
			UART_Send_Enter();

			UART_Send_Str("��Կ�汾��:"); 	
			UART_Put_Num(&g_cReceBuf[9],1);
			UART_Send_Enter();

			UART_Send_Str("�㷨��ʶ:"); 
			UART_Put_Num(&g_cReceBuf[10],1);
			UART_Send_Enter();

			UART_Send_Str("α�����:"); 
			UART_Put_Num(&g_cReceBuf[11],4);
			UART_Send_Enter();
			
			
			
			PAY=g_cReceBuf[2]<<8;		  
	        PAY|=g_cReceBuf[3];	 
		  

 
		
			memcpy(DataBuf, &g_cReceBuf[11], 4);	  //��α�����
			memcpy(&DataBuf[4], &g_cReceBuf[4], 2);	  //�û����������
			memcpy(&DataBuf[6], gPurchase_Pay, 4);	  //���׽��
			DataBuf[10]=0x06;						  //��������
			memcpy(&DataBuf[11], DateTime, 7);	      //��������ʱ��
			DataBuf[18]=DPKey_Flag;                   //������Կ�汾��ʶ��
			DataBuf[19]=0x00;                         //�㷨��ʶ��3DES
			memcpy(&DataBuf[20], S_KEY, 16);	      //��Ա�����б�ʶ���Ӽ������б�ʶ����
			memcpy(&DataBuf[36], CardNum, 8);	      //�û������	


			st=Init_SAM_For_Purchase(DataBuf, 0x2C,  g_cReceBuf); //MAC1�����ʼ��// 
			if(st!=SAM_OK)
			{
				UART_Send_Str("MAC1�������:");       //������9403ʱ����ʱ���һ�����SAM�������������Կ�İ汾��ʶ�ţ��Ƿ��Ӧ��
				UART_Put_Num(g_cReceBuf,0x02);
				UART_Send_Enter();
				continue; 
			}
			
			UART_Send_Str(">>MAC1�����ʼ���ɹ�<<"); 
			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("�ն˽������:"); 
			UART_Put_Num(g_cReceBuf,0x04);
			UART_Send_Enter();		  
			UART_Send_Str("���ص�MAC1��:"); 
			UART_Put_Num(&g_cReceBuf[4],0x04);		  
			UART_Send_Enter();
			

			memcpy(DataBuf, g_cReceBuf, 4); 		 //���ն˽������	
			memcpy(&DataBuf[4], DateTime, 7); 		 //����������ʱ��
		    memcpy(&DataBuf[11], &g_cReceBuf[4], 4); //��MAC1  
		

			st=FM1208_Debit_For_Capp_Purchase(DataBuf,g_cReceBuf,tt); //��������//
			if(st!=0x9000)
			{
				
				if(st!=ST_ERR)
				{
					UART_Send_Str("�ۿ�ʧ��:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);	 //������9302ʱ����ʱ���һ�������Կ�����Ƿ���ȷ,�����û������к�,��Ա���м����д���,������Կ���Ӵ��ˣ���ɢ���Ĺ�����ԿҲ�Ǵ�ģ����Լ������MACҲ�Ǵ��
					UART_Send_Enter();
					 
					main_delay_10ms(200);
				}

				continue; 
			}



				
			UART_Send_Str("�û������صĽ�����֤TAC:"); 
			UART_Put_Num(g_cReceBuf,4);	
			UART_Send_Enter();	  
			UART_Send_Str("�û������ص�MAC1:"); 
			UART_Put_Num(&g_cReceBuf[4],4);		  
			UART_Send_Enter();
			 
			memcpy(DataBuf, &g_cReceBuf[4], 4); 		 //��MAC2,Ȼ���͸�PSAM������У��

		    st=Credit_SAM_For_Purchase(DataBuf, g_cReceBuf);   //MAC2У��//
			if(st!=SAM_OK)
			{
			    
				UART_Send_Str("У��MAC2����:");
				UART_Put_Num(g_cReceBuf,2);   //��ʾ�������
				UART_Send_Enter();
				 
				main_delay_10ms(200);
				continue; 
			}
			

			UART_Send_Str("MAC2У��ۿ�ɹ�!"); 	
 
		 #endif	
/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/
