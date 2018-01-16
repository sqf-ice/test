 
/*==========================================================================*/
/*     I N C L U D E S                                                      */
/*==========================================================================*/
//#include "C_Types.h"
#include "T8029.h" 
#include "stm32f10x.h"
//#include "Serial.h"
//#include "sys_tick_delay.h" 
#include "stm32f10x_dma.h"
#include "atr.h" 
#include "fifo.h" 
#include <string.h >
#include <stdio.h >
#include <stdint.h>
#include <stdbool.h>
#include "fifo.h"
#include "msg_process.h"
#include "psam1.h"

/*==========================================================================*/
/*     L O C A L   S Y M B O L   D E C L A R A T I O N S                    */
/*==========================================================================*/

/*==========================================================================*/
/*     G L O B A L   D E F I N I T I O N S                                  */
/*==========================================================================*/

/*==========================================================================*/
/*     C O N S T A N T   D E F I N I T I O N S                              */
/*==========================================================================*/


/*==========================================================================*/
/*     L O C A L   F U N C T I O N S   P R O T O T Y P E S                  */
/*==========================================================================*/

/*==========================================================================*/
/*     L O C A L   V A R I A B L E S   D E F I N I T I O N S                */
/*==========================================================================*/
 
//void delay(unsigned int d)		// delay in 10 祍
//{
//	unsigned int i;
//	while (d--)
//		for (i = 0; i < 7; i++);
//}

//HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
/*****
 
#define TDA1_INT0_Port 				GPIOB
#define TDA1_INT0_Pin  				GPIO_PIN_0
#define TDA1_INT0_Port 				GPIOB
#define TDA1_INT1_Pin  				GPIO_PIN_1

#define TDA1_P26_Port 				GPIOB
#define TDA1_P26_Pin				GPIO_PIN_5
#define TDA1_RST_Port 				GPIOB
#define TDA1_RST_Pin				GPIO_PIN_8
 
#define TDA1_SDWN_Port 				GPIOB
#define TDA1_SDWN_Pin				GPIO_PIN_9
#define BSP_USART_DMA_CHANNEL	DMA1_Channel5
********/
#define USART3_RX_DMA_CHANNEL 		DMA1_Channel3

fifo_t tda1_fifo,tda2_fifo;
uint8_t gTDA1_Fifobuf[TDA_REC_BUFSIZE];
uint8_t gTDA2_Fifobuf[TDA_REC_BUFSIZE];
uint8_t gTDA1_DMAbuf[TDA_REC_BUFSIZE];
uint8_t gTDA2_DMAbuf[TDA_REC_BUFSIZE];
static void tda_delay_10ms(u16 n10ms)
{
	uint32_t Delaytick;

	Delaytick = BspGetSysTicks();
	while(!BspGetDlyTicks(Delaytick,n10ms));

}
 
 //串口3中断处理函数
void USART3_IRQHandler(void)
{
	uint16_t wlength = 0;
	uint32_t temp =0;
//    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
//    {    
////        TIM_Cmd(TIM2, ENABLE);
////        TIM2->CNT &= 0x0000;    //定时器延时设置：每次接受清零，当接受间隔超过定时器定的值时，进入tim中断，认为一帧接受完成。
////        PN532_RxBuffer[PN532_RXCounter++] = USART_ReceiveData(USART2);
////        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
//    }
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  
	{
		temp = USART3->SR;  
		temp = USART3->DR; // 
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);		
		DMA_Cmd(USART3_RX_DMA_CHANNEL,DISABLE);  
		wlength = (uint16_t)(TDA_REC_BUFSIZE - DMA_GetCurrDataCounter(USART3_RX_DMA_CHANNEL)); 
//		tBspSer.idleflag = 1;
		fifo_push_multiple(&tda1_fifo,gTDA1_DMAbuf,wlength); 
		DMA_SetCurrDataCounter(USART3_RX_DMA_CHANNEL,TDA_REC_BUFSIZE);  
		DMA_Cmd(USART3_RX_DMA_CHANNEL,ENABLE); 
		
	}
	
    /* 发送缓冲区空中断 */
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {   
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    }
}
 void USART3_RX_DMAConfig()
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
	 
	DMA_DeInit(USART3_RX_DMA_CHANNEL);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) gTDA1_DMAbuf ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = TDA_REC_BUFSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(USART3_RX_DMA_CHANNEL,&DMA_InitStructure);

	DMA_Cmd(USART3_RX_DMA_CHANNEL,ENABLE);
	//USART_DMACmd(GPS_USART,USART_DMAReq_Tx,ENABLE);
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);
    
	USART_Cmd(USART3, ENABLE);
	 
 }
void TDA1_Usart3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO , ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=  GPIO_Mode_IN_FLOATING; //GPIO_Mode_IPU
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	/******	*波特率 	************/
	USART_InitStructure.USART_BaudRate            = 38400  ;    
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	//USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	
	USART_ITConfig(USART3, USART_IT_TC, DISABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	USART_ClearFlag(USART3,USART_FLAG_TC);
			
	USART3_RX_DMAConfig();
	
	
}
static void TDA8029_UartBuffer_Config(void)
{
	fifo_init(&tda1_fifo, gTDA1_Fifobuf,TDA_REC_BUFSIZE);
	fifo_init(&tda2_fifo, gTDA2_Fifobuf,TDA_REC_BUFSIZE);
	//HAL_UART_Receive_DMA(&TDA1_UART_Handle,gTDA1_DMAbuf,TDA_REC_BUFSIZE); 
	//HAL_UART_Receive_DMA(&TDA2_UART_Handle,gTDA2_DMAbuf,TDA_REC_BUFSIZE); 
	
}
void TDA8029_IO_Config(void)
{


	GPIO_InitTypeDef GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO , ENABLE );
//	GPIO_InitStructure.GPIO_Pin		= 	TDA1_INT0_Pin;
//	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_10MHz;
//	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IPU;
//	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin		= 	TDA1_RST_Pin;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_Out_PP;
	GPIO_Init(TDA1_RST_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin		= 	TDA1_SDWN_Pin;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_Out_PP;
	GPIO_Init(TDA1_SDWN_Port,&GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin		= 	TDA1_INT0_Pin;
//	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_10MHz;
//	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IPU;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin		= 	TDA1_INT0_Pin;
//	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_10MHz;
//	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IPU;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);
 
	 
	TDA8029_UartBuffer_Config();
	 
} 

 
 
 

 
bool SerialComm_Read (uint8_t *pRdBuffer, uint16_t pRdBytes, uint8_t card_ser)
{
	bool status = false;
	//uint16_t i=0;
	switch(card_ser)
	{
		case TDA_DEVICE1:
			if(fifo_pop_multiple(&tda1_fifo,pRdBuffer,pRdBytes) == FIFO_SUCCESS)
			{
				__nop();
				status = true;
			}
			else
				status = false;
			break;
		case TDA_DEVICE2:
			if(fifo_pop_multiple(&tda2_fifo,pRdBuffer,pRdBytes) == FIFO_SUCCESS)
			{
				__nop();
				status = true;
			}
			else
				status = false;
			break;
		default:
			status = false;
			break;
				
	}
	return status;
	
}

void usart3_send(uint8_t *pBuffer, uint16_t pBytes)
{
	uint16_t i = 0;
	for(i=0;i<pBytes;i++)
	{
		USART_SendData(USART3, pBuffer[i]);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);
	}
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC) == RESET);

	
}
bool SerialComm_Write (uint8_t *pWrBuffer, uint16_t pWrBytes, uint8_t card_ser)
{
	bool status = false;
	switch(card_ser)
	{
		case TDA_DEVICE1:
			usart3_send(pWrBuffer,pWrBytes);
			status = true;
			break;
		case TDA_DEVICE2: 

			break;
		default:
			break;
	}
	return status;
	
}
unsigned char TDA8029_Wakeup(unsigned char *alparBuf, unsigned int  *rBufSize)
{
	unsigned char cmd[16];
	unsigned char rx_buf[32];
	cmd[0] = 0xaa;
	cmd[1] = 0xaa;
	if(SerialComm_Write (cmd, 1, 1) == false)
		return TDA8029_COMMERROR;
	 tda_delay_10ms(1000); 
	if(SerialComm_Read(rx_buf,tda1_fifo.elements_n,1) == true)
	{
		return 0;
	}
	
	return 1;
	
}
/*-------------------------------------------------------------------------------------------------
---- Fonction EmptyHSUrxFIFO
- receive char on UART until it is empty
-------------------------------------------------------------------------------------------------*/
void EmptyHSUrxFIFO (uint32_t mPort)
{
	unsigned char c;
	unsigned int i=1;
	if(mPort == 1)
	{
		//while (SerialComm_Read (&c, i, mPort)); // empty receive FIFO
		tda1_fifo.first = 0;
        tda1_fifo.elements_n = 0;
	}
	else
	{
		tda2_fifo.first = 0;
        tda2_fifo.elements_n = 0;
		__nop();		
	} 
}

/*-------------------------------------------------------------------------------------------------
---- Fonction ReceiveALPARFrame
- reception of a whole ALPAR Frame
- return the status of the reception. 0 = ERROR - 1 = OK.
-------------------------------------------------------------------------------------------------*/
bool ReceiveALPARFrame (unsigned char *pRdBuffer, unsigned int *rlen)
{
	bool status = false;
	int len;
	char checksum = 0x00;

	len = 4; //Get header (0x60) and length (2 bytes)
	status = SerialComm_Read (pRdBuffer, len, 1);
	len = ((*(pRdBuffer+1))<<8) + (*(pRdBuffer+2)) + 1;
	status &= SerialComm_Read (pRdBuffer+4, len, 1);
	*rlen = len + 4;
	return status;
}


/*--------------------------------------------------------------------------------------------------
---- Function to build the ALPAR frame:
     -------------------------------------------------------------------
	 | 60 | lenMSB | lenLSB | AlparCmd |    ... AlparData ...    | CRC |
	 -------------------------------------------------------------------
 ---------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_BuildAlparFrame(unsigned char cmd, unsigned char *pBufferReq, unsigned int BufferReqSize, unsigned char *alparBuf, unsigned int alparBufSize)
{
	unsigned char i, crc = 0;

	unsigned char *tmpAlparCmd = pBufferReq;

	if (BufferReqSize > alparBufSize-5)
		return TDA8029_BUFFERTOOSMALL;

	*alparBuf = ALPAR_PATTERN_OK;	 				// 0x60
	crc ^= *(alparBuf++);
	*alparBuf = (BufferReqSize >> 8) & 0xFF;		// length MSB
	crc ^= *(alparBuf++);
	*alparBuf = BufferReqSize  & 0xFF;				// length LSB
	crc ^= *(alparBuf++);
	*alparBuf = cmd;								// Command Byte
	crc ^= *(alparBuf++);

	for (i=0; i<BufferReqSize; i++)
	{
		*alparBuf = *(pBufferReq++);					// Data
		crc ^= *(alparBuf++);
	}
	*alparBuf = crc;								// Checksum
	
	return TDA8029_OK;
}

/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_ComputeALPARcrc
- Computes the checksum of the alpar frame.
- returns the XOR of all the frame bytes
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_ComputeALPARcrc(unsigned char *alparBuf, unsigned int len)
{
	unsigned int i, crc = 0;
	for (i = 0; i < len; i++)
		crc ^= *(alparBuf++);
	return crc;
}

/*-------------------------------------------------------------------------------------------------
---- Fonction SendAndReceive ALPAR
---- Build the TAMA Frame, sends the Frame and receives the answer
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_ALPAR_SendAndReceive (unsigned char cmd, unsigned char *pBufferReq, unsigned int BufferReqSize, unsigned char *receive_buffer, unsigned int *rlen)
{
	unsigned int i;
	unsigned int len = BufferReqSize + 5;
	unsigned char ALPARCmd[ALPARBUFSIZE];
	unsigned char ALPARAns[ALPARBUFSIZE];
	unsigned char frameCRC, res = 0; 
	static unsigned char tda_timeout = 0;
	EmptyHSUrxFIFO(1); // empty receive FIFO

	res = TDA8029_BuildAlparFrame(cmd, pBufferReq, BufferReqSize, ALPARCmd, ALPARBUFSIZE);
	
	#if EPOS_DEBUG
	// Print the ALPAR frame to be sent
	printf("\n-------------------------\nALPAR Command to be sent:\n --> ");
	for (i = 0; i < BufferReqSize+5; i++)
		printf("%02X ",ALPARCmd[i]);
	printf("\n-------------------------");
	#endif
	// Send the ALPAR Frame
	if(SerialComm_Write (ALPARCmd, len, 1) == false)
		return TDA8029_COMMERROR;
	tda_timeout = 10;
	while(tda_timeout--)
	{
		tda_delay_10ms(2);
		if(tda1_fifo.elements_n > 3)
			break;
	}
	
	// Receive the answer
	if (ReceiveALPARFrame(ALPARAns, &len) == false)
		return TDA8029_COMMERROR;
	#if EPOS_DEBUG
	// Print the received ALPAR Frame
	printf("\n-------------------------\nALPAR Frame received:\n <-- ");
	for (i = 0; i < len; i++)
		printf("%02X ",ALPARAns[i]);
	printf("\n-------------------------\n");
	#endif
	// Check the received ALPAR Frame (Pattern, length, INS byte, CRC...)
	//
	if ((len - 5) > *rlen) return TDA8029_BUFFERTOOSMALL;		// Check if the buffer is big enough for this answer

	frameCRC = TDA8029_ComputeALPARcrc(ALPARAns, len);

	if (ALPARAns[ALPAR_PATTERN_OFFSET] != ALPAR_PATTERN_OK)		// Check if answer starts with 0x60 (means OK)
	{
		if ((ALPARAns[ALPAR_PATTERN_OFFSET] == ALPAR_PATTERN_NOK) &&
			(ALPARAns[ALPAR_MSBLEN_OFFSET] == 0x00) &&
			(ALPARAns[ALPAR_LSBLEN_OFFSET] == 0x01) &&
			(ALPARAns[ALPAR_CMD_OFFSET] == cmd) &&
			(frameCRC == 0))				// Check if the frame is in a correct format starting with E0 (means execution Error, but ALPAR frame is correct)
		{
			*receive_buffer = ALPARAns[ALPAR_DATA_OFFSET];
			*rlen = 1;
			return TDA8029_ALPARPATTERNNOK;						// ALPAR first byte is 0xE0 and Frame is correct
		}
		return TDA8029_ALPARPATTERNERROR;						// ALPAR Pattern (first byte) is not good, or 0xE0 with incorrect frame
	}
	
	if (ALPARAns[ALPAR_CMD_OFFSET] != cmd) return TDA8029_ALPARRESBADCMD;	// The returnde CMD byte is not the same as sent
	if (((ALPARAns[ALPAR_MSBLEN_OFFSET] * 256) + ALPARAns[ALPAR_LSBLEN_OFFSET]) != (len - 5))
		return TDA8029_ALPARBADLENGTH;										// The computed length does not correspond to the received frame

	if (frameCRC != 0) return TDA8029_ALPARBADCHECKSUM;		// The checksum is not good

	// The received ALPAR Frame is correct.
	// Copy the received Data in the returned buffer
	for (i = ALPAR_DATA_OFFSET; i < len - 1; i++)
		*(receive_buffer++) = ALPARAns[i];

	// The data size is the Frame length - 5
	*rlen = len - 5;

	return TDA8029_OK;
}


/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_CheckPluggedDevice
---- Opens the COM port and 
---- checks if the TDA8029 is plugged
-------------------------------------------------------------------------------------------------*/
unsigned char TDA8029_CheckPluggedDevice(void)
{
	unsigned char res = TDA8029_OK;
	unsigned char maskComm[64];
		unsigned int len;
	unsigned char expectedMask[] = "07 Release 1.0";

//	T_RS232_TIMEOUT_DEFAULT = 50;

	// Initialize serial port
 
//		// Send a Mask command to check if the good device has been connected
 		len = sizeof(maskComm);
 		res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_MASK, NULL, 0, maskComm, &len);
 		if(res == TDA8029_OK)
 		{
 			if ((memcmp(expectedMask, maskComm, sizeof(expectedMask)-1)) == 0)
 			{			
				res = TDA8029_OK;
			}
			else
			{
				res = TDA8029_ERROR; 
			}
		}
		else
		{
			res = TDA8029_ERROR;
		}
//	T_RS232_TIMEOUT_DEFAULT = 500;

	return res;
}
extern unsigned char  msg_sendbuf[MAXRLEN];

unsigned char TDA8029_CheckCardIn(void)
{
	unsigned char rBuf[260];
  	unsigned char res = TDA8029_OK;
	unsigned int len, i;
	len = sizeof(rBuf);
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_CHECKCARDPRES, NULL, 0, rBuf, &len);
	if (res == TDA8029_OK)
	{
//		printf("\nCard is ");
//		if (rBuf[0] == 0)
//			printf("ABSENT!\n");
//		else if (rBuf[0] == 1)
//			printf("PRESENT!\n");
//		printf("--------------------\n");
		msg_sendbuf[0] = 0x01;
		msg_sendbuf[1] = rBuf[0];
		msg_pack(UPLOAD_DEVICE_STATUS,msg_sendbuf,2,&g_tMsg );
	}
	return rBuf[0];
			
}
unsigned char TDA8029_CardPowerOn(void)
{
	unsigned char rBuf[260];
	unsigned char pUpData[] = {0x00};
  	unsigned char res = TDA8029_OK;
	unsigned int len, i;
	len = sizeof(rBuf);
	res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_POWERUP3V, pUpData, sizeof(pUpData), rBuf, &len);
	if (res == TDA8029_OK)
	{
		__nop();
		//				ATR_Decode(&tAtr,rBuf,len);
		//				ATR_GetDefaultProtocol(&tAtr,&protocol);
//		printf("\nATR:\n");
//		for (i = 0; i < len; i++)
//		printf("%02X ",rBuf[i]);
//		printf("\n--------------------\n");
	}
	return res;
			
}

/*-------------------------------------------------------------------------------------------------
---- Fonction TDA8029_TestStep
---- Sends a command to the TDA and checks the answer
-------------------------------------------------------------------------------------------------*/
#if 0
//ATR_TypeDef  tAtr;
//int8_t protocol = -1;
void TDA8029_TestStep(unsigned char testNum)
{
	unsigned char rBuf[260];
	unsigned char pUpData[] = {0x00};
	unsigned char sendApduData[] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};	// APDU Select file 3F 00
	//unsigned char sendApduData2[] = {0xa0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};	// APDU Select file 3F 00
	//A0 A4 00 00 02 3F 00
	unsigned char res = TDA8029_OK;
	unsigned int len, i;
	
	switch(testNum)
	{
		case '1':
			len = sizeof(rBuf);
			res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_MASK, NULL, 0, rBuf, &len);
			if (res == TDA8029_OK)
			{
				printf("\nMask version:\n");
				for (i = 0; i < len; i++)
					printf("%c",rBuf[i]);
				printf("\n--------------------\n");
			}
			break;
		case '2':
			len = sizeof(rBuf);
			res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_GETREADERSTATUS, NULL, 0, rBuf, &len);
			if (res == TDA8029_OK)
				printf("\nReader Status : %02X\n", rBuf[0]);
			break;
		case '3':
			len = sizeof(rBuf);
			res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_CHECKCARDPRES, NULL, 0, rBuf, &len);
			if (res == TDA8029_OK)
			{
				printf("\nCard is ");
				if (rBuf[0] == 0)
					printf("ABSENT!\n");
				else if (rBuf[0] == 1)
					printf("PRESENT!\n");
				printf("--------------------\n");
			}
			break;
		case '4':
			len = sizeof(rBuf);
			res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_POWERUP3V, pUpData, sizeof(pUpData), rBuf, &len);
			if (res == TDA8029_OK)
			{
//				ATR_Decode(&tAtr,rBuf,len);
//				ATR_GetDefaultProtocol(&tAtr,&protocol);
				printf("\nATR:\n");
				for (i = 0; i < len; i++)
					printf("%02X ",rBuf[i]);
				printf("\n--------------------\n");
			}
			break;
		case '5':
			len = sizeof(rBuf);
			res = TDA8029_ALPAR_SendAndReceive(ALPAR_CMD_SENDAPDU, sendApduData, sizeof(sendApduData), rBuf, &len);
			if (res == TDA8029_OK)
			{
				printf("\nR-APDU:\n");
				for (i = 0; i < len; i++)
					printf("%02X ",rBuf[i]);
				printf("\n--------------------\n");
			}
			break;
		default:
			res = TDA8029_ERROR;
			break;
	}

	switch (res)
	{
	case TDA8029_ERROR:
		printf("\nERROR - UNKNOWN ERROR \n");
		break;
	case TDA8029_ALPARPATTERNNOK:
		printf("\nERROR - TDA8029 Execution Error: %02X\n", rBuf[0]);
		break;
	case TDA8029_ALPARPATTERNERROR:
		printf("\nERROR - BAD ALPAR PATTERN\n");
		break;
	case TDA8029_ALPARRESBADCMD:
		printf("\nERROR - ALPAR RESPONSE : BAD COMMAND\n");
		break;
	case TDA8029_ALPARBADLENGTH:
		printf("\nERROR - ALPAR RESPONSE : BAD LENGTH\n");
		break;
	case TDA8029_BUFFERTOOSMALL:
		printf("\nERROR - BUFFER TOO SMALL\n");
		break;
	case TDA8029_COMMERROR:
		printf("\nERROR - COMMUNICATION ERROR\n");
		break;
	case TDA8029_ALPARBADCHECKSUM:
		printf("\nERROR - ALPAR BAD CHECKSUM RECEIVED\n");
		break;
	case TDA8029_OK:
	default:
		break;
	}
	//printf("\n> Press ENTER"); 
	 
	 
}
#endif
