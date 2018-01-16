#include "stm32f10x.h"
#include "msg_process.h"
#include "bsp_serial.h" 
#include "stm32f10x_dma.h"
#include <string.h>
#define BSP_USART  				USART1
#define BSP_USART_DMA_CHANNEL	DMA1_Channel5
#define BSP_RECBUFSIZE			64
//u8 bsp_serial_buf[BSP_RECBUFSIZE];


struct BSP_Ser 
{
	//u8 *buf;
	u8 	bsp_serial_buf[BSP_RECBUFSIZE];	
	u16 idleflag;
	u16 wLen;
 	//u16 wStatus; 
};
struct BSP_Ser tBspSer;
void bsp_serial_dma(void);


void bsp_serial_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO , ENABLE );
	 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=  GPIO_Mode_IN_FLOATING; //GPIO_Mode_IPU
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	/********
	*波特率 
	************/
	USART_InitStructure.USART_BaudRate            = 115200  ;    
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(BSP_USART, &USART_InitStructure);
	//USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	
	USART_ITConfig(BSP_USART, USART_IT_TC, DISABLE);
	USART_ITConfig(BSP_USART, USART_IT_RXNE, DISABLE);
	USART_ITConfig(BSP_USART, USART_IT_IDLE, ENABLE);
	USART_ClearFlag(BSP_USART,USART_FLAG_TC);
			
	bsp_serial_dma();
	
	
}
void bsp_serial_dma(void)
{
	DMA_InitTypeDef DMA_InitStructure; 
	//NVIC_InitTypeDef NVIC_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE); 
 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//	 
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
	 
	DMA_DeInit(BSP_USART_DMA_CHANNEL); 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) tBspSer.bsp_serial_buf ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = BSP_RECBUFSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(BSP_USART_DMA_CHANNEL,&DMA_InitStructure);

	DMA_Cmd(BSP_USART_DMA_CHANNEL,ENABLE);
	//USART_DMACmd(GPS_USART,USART_DMAReq_Tx,ENABLE);
	USART_DMACmd(BSP_USART,USART_DMAReq_Rx,ENABLE);
    
	USART_Cmd(BSP_USART, ENABLE);
}


void USART1_IRQHandler(void)
{
 	u8 ucDat;
	u16 temp;
    /* 中断接收*/
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
    { 
		ucDat = USART_ReceiveData(USART1);   
//		g_Usart3RecStr.buf[g_Usart3RecStr.wWritePos++] = ucDat;
//		if(g_Usart3RecStr.wWritePos == USART3_BUFSIZE)
//		{
//			g_Usart3RecStr.wWritePos = 0;
//			g_Usart3RecStr.wRecDataFlag = (g_Usart3RecStr.wRecDataFlag+1) &0xff;
//			
//		}
//		TIM4_Set(1); 
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);        
    }	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
    { 
		temp = USART1->SR;  
		temp = USART1->DR; //?USART_IT_IDLE?? 
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);		
		DMA_Cmd(BSP_USART_DMA_CHANNEL,DISABLE);  
		tBspSer.wLen = BSP_RECBUFSIZE - DMA_GetCurrDataCounter(BSP_USART_DMA_CHANNEL); 
		tBspSer.idleflag = 1;
		if(tBspSer.wLen < MSG_MAXSIZE)
		{
			memcpy(g_tMsg.Data,tBspSer.bsp_serial_buf, tBspSer.wLen);
			DealMsg(&g_tMsg); 
			
		}
		DMA_SetCurrDataCounter(BSP_USART_DMA_CHANNEL,BSP_RECBUFSIZE);  
		DMA_Cmd(BSP_USART_DMA_CHANNEL,ENABLE);  
    }
	
}
void UART_Send_Byte(unsigned char dat)
{
	USART_SendData(USART1, dat);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}
void u8tostr(unsigned char dat) 
{

 switch(dat>>4)	 //高位
 {
 	case 0: UART_Send_Byte(0x30);  break;
	case 1: UART_Send_Byte(0x31);  break;
	case 2: UART_Send_Byte(0x32);  break;
	case 3: UART_Send_Byte(0x33);  break;
	case 4: UART_Send_Byte(0x34);  break;
	case 5: UART_Send_Byte(0x35);  break;
	case 6: UART_Send_Byte(0x36);  break;
	case 7: UART_Send_Byte(0x37);  break;
	case 8: UART_Send_Byte(0x38);  break;
	case 9: UART_Send_Byte(0x39);  break;
	case 0x0A: UART_Send_Byte(0x41);  break;
	case 0x0B: UART_Send_Byte(0x42); break;
	case 0x0C: UART_Send_Byte(0x43); break;
	case 0x0D: UART_Send_Byte(0x44); break;
	case 0x0E: UART_Send_Byte(0x45); break;
	case 0x0F: UART_Send_Byte(0x46); break;

 }

 switch(dat&0x0f)	//低位
 {
 	case 0: UART_Send_Byte(0x30);  break;
	case 1: UART_Send_Byte(0x31);  break;
	case 2: UART_Send_Byte(0x32);  break;
	case 3: UART_Send_Byte(0x33);  break;
	case 4: UART_Send_Byte(0x34);  break;
	case 5: UART_Send_Byte(0x35);  break;
	case 6: UART_Send_Byte(0x36);  break;
	case 7: UART_Send_Byte(0x37);  break;
	case 8: UART_Send_Byte(0x38);  break;
	case 9: UART_Send_Byte(0x39);  break;
	case 0x0A: UART_Send_Byte(0x41);  break;
	case 0x0B: UART_Send_Byte(0x42); break;
	case 0x0C: UART_Send_Byte(0x43); break;
	case 0x0D: UART_Send_Byte(0x44); break;
	case 0x0E: UART_Send_Byte(0x45); break;
	case 0x0F: UART_Send_Byte(0x46); break;

 }

  
}
void UART_Send_Str(char *cBuf)
{
	
	uint16_t i = 0;
	uint16_t len = 0;
	len = strlen(cBuf);
	for(i=0;i<len;i++)
	{
		USART_SendData(USART1, cBuf[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	}
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) == RESET);
	
}
void UART_Send_Enter(void)
{
	USART_SendData(USART1, 0x0d);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, 0x0a);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	
}
void UART_Put_Num(unsigned char * dat,unsigned char len)
{
	unsigned char i;
	
	for(i=0;i<len;i++)
	{
		u8tostr(dat[i]);
		USART_SendData(USART1, 0X20);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	}
	UART_Send_Enter();
}
void UART_Send_Data(unsigned char * dat,unsigned char len)
{
	unsigned char i;
	
	for(i=0;i<len;i++)
	{
		 
		USART_SendData(USART1, dat[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	}
	UART_Send_Enter();
}
unsigned char  SendUartStatus(unsigned int sta)
{
	unsigned char st[2];

	if(sta!=0x9000) 
	{
		 if(sta!=0xFFFF) 
		 {

			 st[0]=sta>>8;
			 st[1]=sta&0x00ff;


			 UART_Send_Str("错误码:");
	
			 UART_Put_Num(st,2);

			// DisCardStatus(t);  //显示屏显示卡状态

		 }
		 else
		 {
			 
		 }

		 return 1;	 //错误
	}
	else
	{
		st[0]=sta>>8;
		st[1]=sta&0x00ff;
		return 0;	//执行指令正确
	}
}
void ProcessBspSer(void)
{
//	T_MSG tMsg; 
//    T_MSG_RECV *ptRecvMsg = (T_MSG_RECV *)&tMsg.Data[0];
//	 
//	if(tBspSer.idleflag )
//	{
//		tMsg.wMsgType = MSG_TYPE_SER;
//		
//		 
//		
//	}
		
}
