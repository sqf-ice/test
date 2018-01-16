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
 //-------------------以下DATA数据，建在MF目录下的所有文件及密钥-----------------------------------------------------//

                              

unsigned char  CardNum[8]; //存放用户卡序列号

unsigned char static  S_KEY[16]={0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; //成员银行标识因子（前8个字节）,试点城市标识因子(后8个字节)




//////////////////文件标识的定义/////////////////////////////////////////////////////////////////////////////////
//
//为统一方便管理阅读代码，我们在所有的文件名后面加个_Flag,表示就是该对应文件的标识//
//
//unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF文件标识3F00
//unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF下的密钥文件标识0000
//unsigned char static ExKey_Flag       = {0x00};           //MF下的外部密钥标识00
//unsigned char static LineKey_Flag     = {0x01}; 		    //MF下的线路保护密钥标识01
//unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF下的定长记录文件标识0001
//unsigned char static RECORD_Flag      = {0x08}; 		    //MF下的定长记录标识08

//unsigned char static CZKey_Flag={0x01};	  //圈存密钥标识为01
unsigned char static DPKey_Flag={0x00};	  //消费密钥标识为00
unsigned char static DF_ExKey_Flag  = {0x00};      //DF下的外部密钥标识00
unsigned char static DF_Inital_Key_Flag  = {0x00}; //DF下的内部密钥标识00


//unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF下的口令PIN密钥标识00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//公共应用基本数据文件标识0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//持卡人基本信息数据的文件标识0016
//unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //交易明细文件标识0018
//unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//电子存折文件标识0001
//unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//电子钱包文件标识0002




////////////////////////////////////////////////////////////////////////
//文件标识

 
unsigned char static File_0016_s={0x16};	        //终端机短文件标识

unsigned char  static     DateTime[]={0x20,0x17,0x10,0x26,0x11,0x32,0x45};
unsigned char             gRecDateTime[7];
unsigned char      Temporary_Key[8];  //存放过程密钥
unsigned char      Old_Pay[4];        //存放旧钱包余额，即未操作前的余额。
unsigned char      Challenge_Num[8];  //随机数
unsigned char      Serial_Num[2];     //交易序号
unsigned char      MAC1_Num[4];       //MAC1值

unsigned char      NameBuf[20];	 //临时存放姓名
unsigned char      NameLen;		 //姓名长度

unsigned char  gPurchase_Pay[4]={0x00,0x00,0x00,0x01}; //操作的金额 高位在前，16进制计算的，定额消费1元
uint32_t gPurchaseNum;
unsigned char  gPurchaseFlag =0;
unsigned int PAY;



unsigned char  POSNum[6]; 								  //终端机编号	从SAM卡中读取
 
static void RCC_Configuration(void)
{
	//参考网址：http://blog.csdn.net/iamlvshijie/article/details/9249545
	ErrorStatus HSEStartUpStatus; 
	RCC_DeInit();     //设置RCC寄存器重新设置为默认值
	RCC_HSEConfig (RCC_HSE_ON);//打开外部高速时钟晶振（HSE）
	HSEStartUpStatus = RCC_WaitForHSEStartUp();//等待外部时钟晶振稳定工作
	if(HSEStartUpStatus == SUCCESS)//SUCCESS：HSE晶振稳定且就绪
	{
		//FLASH_PrefetchBufferCmd(ENABLE);          //开启FLASH的预取功能
		//FLASH_SetLatency(FLASH_Latency_2);      //FLASH延长两个周期
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//设置AHB时钟不分频
		RCC_PCLK2Config(RCC_HCLK_Div1); //设置APB2时钟不分频
		RCC_PCLK1Config(RCC_HCLK_Div2); //设置APB1时钟二分频 对应的定时器的时钟为2倍频
//	FLASH_SetLatency(FLASH_Latency_2);//设置FLASH存储器延时周期数，FLASH_Latency_2 2延时周期
//	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//选择FLASH预取指缓存的模式，预取指缓存使能
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9);//设置PLL   
		RCC_PLLCmd(ENABLE); //PLL时钟使能
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while(RCC_GetSYSCLKSource()!=0x08);//等待工作。
	}
	else
	{
		__nop();
	}
//	RCC_LSICmd(ENABLE);
//  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
// 
	//开启输入端口时钟
	//使能PA~PE端口时钟IN1~IN32
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 
//	//开启串口1、2、3时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART1时钟
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2，USART3时钟
//	
	/* Configure HCLK clock as SysTick clock source. */
	//SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
 
}
 
/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
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
 //计算姓名的长度，及临时保存姓名
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
* 函数功能 ---> 读取CPU 96位唯一ID
* 入口参数 --->  
* 返回数值 ---> 
* 功能说明 ---> none
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

	/*配置PN532	 Wired Card, the host controller can access to the SAM with standard PCD commands (InListPassiveTarget,InDataExchange, …)*/
 	
	
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
			continue; //寻卡
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
					st=PSAM_Select_File(File_3F00, 2, 0,g_cReceBuf, &gRepLen);// 0x28 );	//选择SAM卡中PBOC的3F00文件// 28
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM卡3F00文件打开错误:");
						UART_Put_Num(g_cReceBuf,16);   //显示错误代码
						UART_Send_Enter();	
						#endif
					   gCpuReadState = 0; break;//continue;
				
					}
					gCpuReadState ++;
				case READ_SAM_POSNUM:
					gRepLen = 6;
					st=PSAM_ReadBinary( File_0016_s,g_cReceBuf,&gRepLen); //  6 );	 //读取SAM卡里面的终端机编号//
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM卡的终端机编号读取错误:");
						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
						UART_Send_Enter();
						#endif
   					   gCpuReadState = 0; break;//continue;
				
					}
					#if EPOS_DEBUG
					UART_Send_Str("终端机编号为:");						 
					UART_Put_Num(g_cReceBuf,6); UART_Send_Enter();
					#endif
					memcpy(POSNum, g_cReceBuf, 6);
					gCpuReadState ++;
				case OPEN_SAM_FILE3F01:
					st=PSAM_Select_File(File_3F01, 2, 0,g_cReceBuf,&gRepLen); // 0x27 );	//选择SAM卡中PBOC的3F01文件
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG
						UART_Send_Str("SAM卡PBOC文件打开错误:");
						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
						UART_Send_Enter();			   
						#endif						
					   gCpuReadState = 0; break;//continue;
				
					}					 
					#if EPOS_DEBUG
					UART_Send_Str("SAM卡PBOC文件打开成功!");
					UART_Send_Enter();
					UART_Send_Enter();
					#endif
					gCpuReadState ++; //break;
				case OPEN_CPU_FILE3F01:
					st=FM1208_SelectFile(File_3F01,g_cReceBuf,tt);      //打开CPU卡中PBOC的3F01文件//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							UART_Send_Str("用户卡PBOC文件打开错误:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);
							UART_Send_Enter();
						}
						 gCpuReadState = 0; break;//continue;continue; 
					}
					#if EPOS_DEBUG 
						UART_Send_Str(">>用户卡PBOC文件打开成功<<"); UART_Send_Enter();
					#endif
					gCpuReadState ++;
				case READ_CPU_CARDNUM:					
					st=FM1208_Read_Binary(FM1208_DF_0015_Flag[1],g_cReceBuf, 0x1E );		  //读取0015文件公共应用基本数据//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							UART_Send_Str("公共应用基本文件读取失败:"); 
							UART_Put_Num(g_cReceBuf,2);
							UART_Send_Enter();
						}
						 gCpuReadState = 0; break;//continue;continue; 
					}
					#if EPOS_DEBUG 
					 UART_Send_Str(">>公共应用基本文件读取成功<<"); 
					 UART_Send_Enter();					 UART_Send_Enter();
					#endif
					
     				 memcpy(CardNum, &g_cReceBuf[12], 8);   //拷入用户卡序号
					#if EPOS_DEBUG 
					 UART_Send_Str("用户卡序号:"); 
					 UART_Put_Num(CardNum,8);				//用户卡序号
					 UART_Send_Enter();
					#endif
					gCpuReadState ++; //					break;
	
				case DES_INIT_SAM_CPU_EXTH:					
//					st=FM1208_GetChallenge(8,g_cReceBuf);			   //从用户卡取一组随机数//
//					if(st!=0x9000)
//					{	
//						if(st!=ST_ERR)
//						{
//							#if EPOS_DEBUG 
//							UART_Send_Str("用户卡读取随机码错误:"); 
//							UART_Put_Num(g_cReceBuf,2);
//							UART_Send_Enter();
//							#endif
//						}
//						 gCpuReadState = 0; break;//continue;continue; 
//					}
//					memcpy(Challenge_Num, g_cReceBuf, 8);     //临时保存随机数
//					st=Init_for_descrypt(CardNum, 8, 0x27,DF_ExKey_Flag, g_cReceBuf);     //用3DE初始化 1级分散产生过程密钥    27--密钥用途,为加密密钥     
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 

//						UART_Send_Str("SAM卡的3DES初始化失败:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();
//						#endif						
//						 gCpuReadState = 0; break;// 
//					}
					gCpuReadState ++;
				case DES_CALC_SAM_CPU_EXTH:					 
//					st=PSAM_Descrypt(Challenge_Num, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES算法 对刚才临时保存的随机数进行加密//
//					if(st!=SAM_OK)
//					{
//						#if EPOS_DEBUG 

//						UART_Send_Str("SAM卡的3DES计算错误:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;
//					}
//					#if EPOS_DEBUG 

//				//	 Single_3DES_Encrypt(Challenge_Num,cpu_ex_key,g_cReceBuf);
//					UART_Send_Str("外部认证的加密结果:"); 
//					UART_Put_Num(g_cReceBuf,8);	UART_Send_Enter();
//					#endif
//					
//					memcpy(DataBuf, g_cReceBuf, 8);	  //临时保存加密结果
					gCpuReadState ++;
				case EXTH_CPU_AUTH:
					
//					st=FM1208_External_Authenticate(DF_ExKey_Flag,DataBuf,g_cReceBuf);  //执行外部认证//
//					if(st!=0x9000)
//					{	
//						if(st!=ST_ERR)
//						{
//							#if EPOS_DEBUG 

//							UART_Send_Str("终端POS机非法,外部认证错误:"); 
//							UART_Put_Num(g_cReceBuf,2);
//							UART_Send_Enter();
//							#endif
//						}
//						gCpuReadState = 0; break;//continue;while(1);
//						 
//					}
//					#if EPOS_DEBUG  
//					UART_Send_Str("外部认证成功,终端POS机合法!"); 
//					UART_Send_Enter();
//					UART_Send_Enter();
//					#endif
					gCpuReadState ++;
			case DES_RAND_CPU_INTH:
//					st=PSAM_Get_challenge(8,g_cReceBuf);		       //从SAM卡里取一组随机数//    
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM卡读取随机码错误:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}
//					memcpy(Challenge_Num, g_cReceBuf, 8);     //临时保存随机数
//			 		st=FM1208_Inital_Authenticate(DF_Inital_Key_Flag, 8, Challenge_Num,g_cReceBuf); //用户卡内部认证,加密该随机数//
//					if(st!=0x9000)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("内部认证错误:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();			   					            
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}
//					memcpy(DataBuf, g_cReceBuf, 8);	  //临时保存加密结果
					gCpuReadState ++;		 
				case DES_INIT_SAM_CPU_INTH:
//					st=Init_for_descrypt(CardNum, 0x8, 0x2C, DF_Inital_Key_Flag, g_cReceBuf);     //用3DE初始化 1级分散产生过程密钥    2C--密钥用途,为解密密钥  
//					if(st!=SAM_OK)								 
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM卡的3DES初始化失败:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;		
//					}					
					gCpuReadState ++;
				case DES_CALC_SAM_CPU_INTH:
//					st=PSAM_Descrypt(DataBuf, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES算法 对刚才临时保存的加密了的数据进行解密//
//					if(st!=SAM_OK)
//					{
//						#if EPOS_DEBUG 
//						UART_Send_Str("SAM卡的3DES计算错误:");
//						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
//						UART_Send_Enter();	
//						#endif
//						gCpuReadState = 0; break;//continue;while(1);continue;
//					}
//					#if EPOS_DEBUG 

//					UART_Send_Str("内部认证的解密结果:"); 
//					UART_Put_Num(g_cReceBuf,8);	
//					UART_Send_Enter();
//					#endif
//				
//					for(i=0;i<8;i++)
//					{
//						if(g_cReceBuf[i]!=Challenge_Num[i])		//这里判断卡的合法性//
//						{
//						  // send to epos
//							
//						  #if EPOS_DEBUG 
//						  UART_Send_Str("非法卡!不能在本终端使用!");
//						  UART_Send_Enter();
//						  #endif
//						  
//						  gCpuReadState = 0; break;//continue;while(1);continue;				
//						}
//					}
//					#if EPOS_DEBUG 
//						UART_Send_Str("内部认证成功,用户卡合法!");
//						UART_Send_Enter();					UART_Send_Enter();
//					#endif
					gCpuReadState ++;
				case READ_CPU_FILE0016_INF:	
 					st=FM1208_Read_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, 0x27 );		  //读取0016文件持卡人基本信息
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("持卡人基本信息文件读取失败:"); 
							UART_Put_Num(g_cReceBuf,2);
							UART_Send_Enter();
							#endif
						}
						gCpuReadState = 0; break;//continue;while(1);continue;				
 
					}
					#if EPOS_DEBUG 

					UART_Send_Str(">>持卡人基本信息文件读取成功<<"); 
					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("卡类型标识:"); 
					UART_Put_Num(&g_cReceBuf[0],1);
					UART_Send_Enter();

					UART_Send_Str("本行职工标识:"); 
					UART_Put_Num(&g_cReceBuf[1],1);
					UART_Send_Enter();

					UART_Send_Str("持 卡 人 的姓名:"); 
					UART_Send_Str((char*)&g_cReceBuf[2]); 
					NameToStr(&g_cReceBuf[2]);


					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("持 卡 人 的证件类型:"); 
					UART_Put_Num(&g_cReceBuf[38],1);
					UART_Send_Enter();

					UART_Send_Str("持 卡 人 的证件号码:"); 
					UART_Put_Num(&g_cReceBuf[22],16);
					UART_Send_Enter();
					#endif
					
					DataBuf[0]=DPKey_Flag;	              //1个字节的消费密钥标识					
					memcpy(&DataBuf[1], gPurchase_Pay, 4); //4个字节的交易金额，即扣款金额	
					memcpy(&DataBuf[5], POSNum, 6);	      //6个字节的终端机编号
					gCpuReadState ++;
		
			
				case INIT_PURCH_CPU:
					st=FM1208_Initalize_For_Purchase(EP, DataBuf,g_cReceBuf,tt); //消费初始化操作//
					if(st!=0x9000)
					{	
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("初始化消费错误:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);	  //当返回9403时，此时检查一下你的用户卡里面的消费密钥的标识号，是否对应？
							UART_Send_Enter();
							#endif
							if(st==0x9401)
							{
							  //DisNotMoney();	 //卡余额不足
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
					

					UART_Send_Str(">>消费初始化成功<<"); 
					UART_Send_Enter();
					UART_Send_Enter();
										 
					UART_Send_Str("电子钱包未消费前的余额:"); 
					UART_Put_Num(g_cReceBuf,4);
					UART_Send_Enter();

					UART_Send_Str("电子钱 包 交 易序号:"); 
					UART_Put_Num(&g_cReceBuf[4],2);
					UART_Send_Enter();

					UART_Send_Str(" 限 额:"); 
					UART_Put_Num(&g_cReceBuf[6],3);
					UART_Send_Enter();

					UART_Send_Str("密钥版本号:"); 	
					UART_Put_Num(&g_cReceBuf[9],1);
					UART_Send_Enter();

					UART_Send_Str("算法标识:"); 
					UART_Put_Num(&g_cReceBuf[10],1);
					UART_Send_Enter();

					UART_Send_Str("伪随机码:"); 
					UART_Put_Num(&g_cReceBuf[11],4);
					UART_Send_Enter();
					#endif	
					
//					UART_Send_Str("电子钱包未消费前的余额:"); 
//					UART_Put_Num(g_cReceBuf,4);
//					UART_Send_Enter();
					memcpy(&msg_sendbuf[8],g_cReceBuf,4);
					memcpy(msg_sendbuf,CardNum,8);
					msg_pack(UPLOAD_REMAINING_MONEY,msg_sendbuf,4+8,&g_tMsg );
					
					PAY=g_cReceBuf[2]<<8;		  
					PAY|=g_cReceBuf[3];					
					memcpy(DataBuf, &g_cReceBuf[11], 4);	  //拷伪随机数
					memcpy(&DataBuf[4], &g_cReceBuf[4], 2);	  //用户卡交易序号
					memcpy(&DataBuf[6], gPurchase_Pay, 4);	  //交易金额
					DataBuf[10]=0x06;						  //交易类型
					memcpy(&DataBuf[11], DateTime, 7);	      //交易日期时间
					 
					DataBuf[18]=DPKey_Flag;                   //消费密钥版本标识号
					DataBuf[19]=0x00;                         //算法标识，3DES
					memcpy(&DataBuf[20], S_KEY, 16);	      //成员各银行标识因子及各城市标识因子
					memcpy(&DataBuf[36], CardNum, 8);	      //用户卡序号	
					gPurchaseNum = (gPurchase_Pay[0] << 24) + (gPurchase_Pay[1] << 16)+(gPurchase_Pay[2] << 8) +gPurchase_Pay[3];
					if((gPurchaseFlag ==1)&& (gPurchaseNum > 0))
					{
						gCpuReadState ++;
					}
					else
						break;
		 
				case INIT_PURCH_PSAM:
				////////////////////////////////////////////////////////////////////			  
					st=Init_SAM_For_Purchase(DataBuf, 0x2C,  g_cReceBuf); //MAC1计算初始化// 
					if(st!=SAM_OK)
					{
						#if EPOS_DEBUG 
						UART_Send_Str("MAC1计算错误:");       //当返回9403时，此时检查一下你的SAM卡里面的消费密钥的版本标识号，是否对应？
						UART_Put_Num(g_cReceBuf,0x02);						UART_Send_Enter();
						#endif
						gCpuReadState = 0; break;//continue;while(1);continue;				 
					}
					#if EPOS_DEBUG 
					UART_Send_Str(">>MAC1计算初始化成功<<"); 
					UART_Send_Enter();
					UART_Send_Enter();

					UART_Send_Str("终端交易序号:"); 
					UART_Put_Num(g_cReceBuf,0x04);
					UART_Send_Enter();		  
					UART_Send_Str("返回的MAC1码:"); 
					UART_Put_Num(&g_cReceBuf[4],0x04);		  
					UART_Send_Enter();
					#endif

					memcpy(DataBuf, g_cReceBuf, 4); 		 //拷终端交易序号	
					memcpy(&DataBuf[4], DateTime, 7); 		 //拷交易日期时间
					memcpy(&DataBuf[11], &g_cReceBuf[4], 4); //拷MAC1  
					gCpuReadState ++;
				case DB_PURCH_CPU:					
					st=FM1208_Debit_For_Capp_Purchase(DataBuf,g_cReceBuf,tt); //消费命令//
					if(st!=0x9000)
					{
						
						if(st!=ST_ERR)
						{
							#if EPOS_DEBUG 

							UART_Send_Str("扣款失败:"); 
							UART_Put_Num(g_cReceBuf,tt[0]);	 //当返回9302时，此时检查一下你的密钥因子是否正确,包括用户卡序列号,成员银行及城市代码,由于密钥因子错了，分散出的过程密钥也是错的，所以计算出的MAC也是错的
							UART_Send_Enter();
							 #endif
							//main_delay_10ms(200);
						}
						gCpuReadState = 0; break;//continue; 
					}
					#if EPOS_DEBUG 
					
					UART_Send_Str("用户卡返回的交易验证TAC:"); 
					UART_Put_Num(g_cReceBuf,4);	
					UART_Send_Enter();	  
					UART_Send_Str("用户卡返回的MAC1:"); 
					UART_Put_Num(&g_cReceBuf[4],4);		  
					UART_Send_Enter();
					 #endif
					memcpy(DataBuf, &g_cReceBuf[4], 4); 		 //拷MAC2,然后发送给PSAM卡进行校验
				case CHECK_MAC2_PSAM:
					st=Credit_SAM_For_Purchase(DataBuf, g_cReceBuf);   //MAC2校验//
					if(st!=SAM_OK)
					{
						
						UART_Send_Str("校验MAC2错误:");
						UART_Put_Num(g_cReceBuf,2);   //显示错误代码
						UART_Send_Enter();
						 
						//main_delay_10ms(200);
					  gCpuReadState = 0; break;//continue;while(1);continue; continue; 
					}
					//UART_Send_Str("MAC2校验扣款成功!"); 
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


//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

		#if 0
		if(!nfc_InListPassiveTarget(0x01,0,UID))
			{
				continue; //寻卡
			}
			
			gRepLen = 0x28 ;			
		    st=PSAM_Select_File(File_3F00, 2, 0,g_cReceBuf, &gRepLen);// 0x28 );	//选择SAM卡中PBOC的3F00文件// 28
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM卡3F00文件打开错误:");
				UART_Put_Num(g_cReceBuf,16);   //显示错误代码
				UART_Send_Enter();			   
			   continue;
		
		    }
			gRepLen = 6;
			st=PSAM_ReadBinary( File_0016_s,g_cReceBuf,&gRepLen); //  6 );	 //读取SAM卡里面的终端机编号//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM卡的终端机编号读取错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   
			   continue;
		
		    }
			UART_Send_Str("终端机编号为:");
			memcpy(POSNum, g_cReceBuf, 6); 	 
			UART_Put_Num(POSNum,6); UART_Send_Enter();

		    st=PSAM_Select_File(File_3F01, 2, 0,g_cReceBuf,&gRepLen); // 0x27 );	//选择SAM卡中PBOC的3F01文件
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM卡PBOC文件打开错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   
			   					   	while(1);		            
		            
			   continue;
		
		    }
			UART_Send_Str("SAM卡PBOC文件打开成功!");
			UART_Send_Enter();
			UART_Send_Enter();
 			
			st=FM1208_SelectFile(File_3F01,g_cReceBuf,tt);      //打开CPU卡中PBOC的3F01文件//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("用户卡PBOC文件打开错误:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);
					UART_Send_Enter();
				}
							   	while(1);		            

				continue; 
			}
			UART_Send_Str(">>用户卡PBOC文件打开成功<<"); 
			UART_Send_Enter();
			UART_Send_Enter();
 

			st=FM1208_Read_Binary(FM1208_DF_0015_Flag[1],g_cReceBuf, 0x1E );		  //读取0015文件公共应用基本数据//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("公共应用基本文件读取失败:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
			   	while(1);		            

				continue; 
			}
			 UART_Send_Str(">>公共应用基本文件读取成功<<"); 
			 UART_Send_Enter();
			 UART_Send_Enter();
		    
			 memcpy(CardNum, &g_cReceBuf[12], 8);   //拷入用户卡序号
			 UART_Send_Str("用户卡序号:"); 
			 UART_Put_Num(CardNum,8);				//用户卡序号
			 UART_Send_Enter();
			  
			//////////////////////////////////////////////////////////////
			//主要是用户卡判断终端的合法性，即是否是有人伪造的终端机
			
			st=FM1208_GetChallenge(8,g_cReceBuf);			   //从用户卡取一组随机数//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("用户卡读取随机码错误:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
			   	while(1);		            

				continue; 
			}
			memcpy(Challenge_Num, g_cReceBuf, 8);     //临时保存随机数
			
				st=Init_for_descrypt(CardNum, 8, 0x27,DF_ExKey_Flag, g_cReceBuf);     //用3DE初始化 1级分散产生过程密钥    27--密钥用途,为加密密钥     
				if(st!=SAM_OK)								 
				{
					// UART_Send_Str("SAM卡的3DES初始化失败:");
					UART_Put_Num(g_cReceBuf,2);   //显示错误代码
					UART_Send_Enter();	
					// while(1);
					//continue;		
				}
	 
		 
			// while(1);
			st=PSAM_Descrypt(Challenge_Num, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES算法 对刚才临时保存的随机数进行加密//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM卡的3DES计算错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();	
			   	while(1);		            
				
			    continue;
		    }
		//	 Single_3DES_Encrypt(Challenge_Num,cpu_ex_key,g_cReceBuf);
			UART_Send_Str("外部认证的加密结果:"); 
			UART_Put_Num(g_cReceBuf,8);	
			memcpy(DataBuf, g_cReceBuf, 8);	  //临时保存加密结果
			UART_Send_Enter();

			//while(1);
			st=FM1208_External_Authenticate(DF_ExKey_Flag,DataBuf,g_cReceBuf);  //执行外部认证//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("终端POS机非法,外部认证错误:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}
				while(1);
				continue; 
			}

			UART_Send_Str("外部认证成功,终端POS机合法!"); 
			UART_Send_Enter();
			UART_Send_Enter();
			 
			////////////////////////////////////////////////////////////////////
			 
			//////////////////////////////////////////////////////////////
			//主要用于终端判断卡的合法性，也就是说你这张用户卡是否能在我的终端上使用
		    st=PSAM_Get_challenge(8,g_cReceBuf);		       //从SAM卡里取一组随机数//    
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM卡读取随机码错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   					            
			    continue;		
		    }
			memcpy(Challenge_Num, g_cReceBuf, 8);     //临时保存随机数

			
			st=FM1208_Inital_Authenticate(DF_Inital_Key_Flag, 8, Challenge_Num,g_cReceBuf); //用户卡内部认证,加密该随机数//
		    if(st!=0x9000)								 
		    {
			    UART_Send_Str("内部认证错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   					            
			    continue;		
		    }
			memcpy(DataBuf, g_cReceBuf, 8);	  //临时保存加密结果

			
			st=Init_for_descrypt(CardNum, 0x8, 0x2C, DF_Inital_Key_Flag, g_cReceBuf);     //用3DE初始化 1级分散产生过程密钥    2C--密钥用途,为解密密钥  
		    if(st!=SAM_OK)								 
		    {
			    UART_Send_Str("SAM卡的3DES初始化失败:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   					            
			    continue;		
		    }
	
			st=PSAM_Descrypt(DataBuf, 0x8, 0, g_cReceBuf, 0x8 );	   //3DES算法 对刚才临时保存的加密了的数据进行解密//
		    if(st!=SAM_OK)
		    {
			    UART_Send_Str("SAM卡的3DES计算错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();			   					            
			    continue;
		    }

			UART_Send_Str("内部认证的解密结果:"); 
			UART_Put_Num(g_cReceBuf,8);	
			UART_Send_Enter();


			for(i=0;i<8;i++)
			{
				if(g_cReceBuf[i]!=Challenge_Num[i])		//这里判断卡的合法性//
				{
				  UART_Send_Str("非法卡!不能在本终端使用!");
				  UART_Send_Enter();
				  continue;				
				}
			}

		    UART_Send_Str("内部认证成功,用户卡合法!");
		    UART_Send_Enter();
			UART_Send_Enter();
			////////////////////////////////////////////////////////////////////			  
			  
			  
			  
			  
			   
			
			st=FM1208_Read_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, 0x27 );		  //读取0016文件持卡人基本信息
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("持卡人基本信息文件读取失败:"); 
					UART_Put_Num(g_cReceBuf,2);
					UART_Send_Enter();
				}

				continue; 
			}

			UART_Send_Str(">>持卡人基本信息文件读取成功<<"); 
			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("卡类型标识:"); 
			UART_Put_Num(&g_cReceBuf[0],1);
			UART_Send_Enter();

			UART_Send_Str("本行职工标识:"); 
			UART_Put_Num(&g_cReceBuf[1],1);
			UART_Send_Enter();

			UART_Send_Str("持 卡 人 的姓名:"); 
			UART_Send_Str((char*)&g_cReceBuf[2]); 
			NameToStr(&g_cReceBuf[2]);


			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("持 卡 人 的证件类型:"); 
			UART_Put_Num(&g_cReceBuf[38],1);
			UART_Send_Enter();

			UART_Send_Str("持 卡 人 的证件号码:"); 
			UART_Put_Num(&g_cReceBuf[22],16);
			UART_Send_Enter();


			
			DataBuf[0]=DPKey_Flag;	              //1个字节的消费密钥标识					
			memcpy(&DataBuf[1], gPurchase_Pay, 4); //4个字节的交易金额，即扣款金额	
			memcpy(&DataBuf[5], POSNum, 6);	      //6个字节的终端机编号


		    st=FM1208_Initalize_For_Purchase(EP, DataBuf,g_cReceBuf,tt); //消费初始化操作//
			if(st!=0x9000)
			{	
				if(st!=ST_ERR)
				{
					UART_Send_Str("初始化消费错误:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);	  //当返回9403时，此时检查一下你的用户卡里面的消费密钥的标识号，是否对应？
					UART_Send_Enter();

					if(st==0x9401)
					{
					  //DisNotMoney();	 //卡余额不足
					}
					main_delay_10ms(2000);
					 
				}

				continue; 
			}
			
			UART_Send_Str(">>消费初始化成功<<"); 
			UART_Send_Enter();
			UART_Send_Enter();
								 
			UART_Send_Str("电子钱包未消费前的余额:"); 
			UART_Put_Num(g_cReceBuf,4);
			UART_Send_Enter();

			UART_Send_Str("电子钱 包 交 易序号:"); 
			UART_Put_Num(&g_cReceBuf[4],2);
			UART_Send_Enter();

			UART_Send_Str(" 限 额:"); 
			UART_Put_Num(&g_cReceBuf[6],3);
			UART_Send_Enter();

			UART_Send_Str("密钥版本号:"); 	
			UART_Put_Num(&g_cReceBuf[9],1);
			UART_Send_Enter();

			UART_Send_Str("算法标识:"); 
			UART_Put_Num(&g_cReceBuf[10],1);
			UART_Send_Enter();

			UART_Send_Str("伪随机码:"); 
			UART_Put_Num(&g_cReceBuf[11],4);
			UART_Send_Enter();
			
			
			
			PAY=g_cReceBuf[2]<<8;		  
	        PAY|=g_cReceBuf[3];	 
		  

 
		
			memcpy(DataBuf, &g_cReceBuf[11], 4);	  //拷伪随机数
			memcpy(&DataBuf[4], &g_cReceBuf[4], 2);	  //用户卡交易序号
			memcpy(&DataBuf[6], gPurchase_Pay, 4);	  //交易金额
			DataBuf[10]=0x06;						  //交易类型
			memcpy(&DataBuf[11], DateTime, 7);	      //交易日期时间
			DataBuf[18]=DPKey_Flag;                   //消费密钥版本标识号
			DataBuf[19]=0x00;                         //算法标识，3DES
			memcpy(&DataBuf[20], S_KEY, 16);	      //成员各银行标识因子及各城市标识因子
			memcpy(&DataBuf[36], CardNum, 8);	      //用户卡序号	


			st=Init_SAM_For_Purchase(DataBuf, 0x2C,  g_cReceBuf); //MAC1计算初始化// 
			if(st!=SAM_OK)
			{
				UART_Send_Str("MAC1计算错误:");       //当返回9403时，此时检查一下你的SAM卡里面的消费密钥的版本标识号，是否对应？
				UART_Put_Num(g_cReceBuf,0x02);
				UART_Send_Enter();
				continue; 
			}
			
			UART_Send_Str(">>MAC1计算初始化成功<<"); 
			UART_Send_Enter();
			UART_Send_Enter();

			UART_Send_Str("终端交易序号:"); 
			UART_Put_Num(g_cReceBuf,0x04);
			UART_Send_Enter();		  
			UART_Send_Str("返回的MAC1码:"); 
			UART_Put_Num(&g_cReceBuf[4],0x04);		  
			UART_Send_Enter();
			

			memcpy(DataBuf, g_cReceBuf, 4); 		 //拷终端交易序号	
			memcpy(&DataBuf[4], DateTime, 7); 		 //拷交易日期时间
		    memcpy(&DataBuf[11], &g_cReceBuf[4], 4); //拷MAC1  
		

			st=FM1208_Debit_For_Capp_Purchase(DataBuf,g_cReceBuf,tt); //消费命令//
			if(st!=0x9000)
			{
				
				if(st!=ST_ERR)
				{
					UART_Send_Str("扣款失败:"); 
					UART_Put_Num(g_cReceBuf,tt[0]);	 //当返回9302时，此时检查一下你的密钥因子是否正确,包括用户卡序列号,成员银行及城市代码,由于密钥因子错了，分散出的过程密钥也是错的，所以计算出的MAC也是错的
					UART_Send_Enter();
					 
					main_delay_10ms(200);
				}

				continue; 
			}



				
			UART_Send_Str("用户卡返回的交易验证TAC:"); 
			UART_Put_Num(g_cReceBuf,4);	
			UART_Send_Enter();	  
			UART_Send_Str("用户卡返回的MAC1:"); 
			UART_Put_Num(&g_cReceBuf[4],4);		  
			UART_Send_Enter();
			 
			memcpy(DataBuf, &g_cReceBuf[4], 4); 		 //拷MAC2,然后发送给PSAM卡进行校验

		    st=Credit_SAM_For_Purchase(DataBuf, g_cReceBuf);   //MAC2校验//
			if(st!=SAM_OK)
			{
			    
				UART_Send_Str("校验MAC2错误:");
				UART_Put_Num(g_cReceBuf,2);   //显示错误代码
				UART_Send_Enter();
				 
				main_delay_10ms(200);
				continue; 
			}
			

			UART_Send_Str("MAC2校验扣款成功!"); 	
 
		 #endif	
/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/
