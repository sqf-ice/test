/*_BEGIN_FILE_HEADER_*******************************************************/
/*                                                                         */
/*  FILE NAME       : T8029.h                                             */
/*                                                                         */
/*  FILE LOCATION   : -                                                    */
/*                                                                         */
/*  LANGUAGE        : C                                                    */
/*                                                                         */
/*  PORTABILITY     : ANSI                                                 */
/*                                                                         */
/*  DESCRIPTION     : TDA8029 specific functions header file               */
/*                                                                         */
/*  ENVIRONMENT     : Cake80xx_MBA Board                                   */
/*                                                                         */
/*  DOCUMENTATION   :                                                      */
/*                                                                         */
/*  MODIFICATIONS   :                                                      */
/* _______________________________________________________________________ */
/*|        |          |       |                                           |*/
/*|  DATE  |  AUTHOR  |VERSION|           DESCRIPTION                     |*/
/*|________|__________|_______|___________________________________________|*/
/*|17/11/09|C LOMBARDO|  1.0  | CREATION                                  |*/
/*|________|__________|_______|___________________________________________|*/
/*|        |          |       |                                           |*/
/*|________|__________|_______|___________________________________________|*/
/*                                                                         */
/*  Copyright (c) 2009, NXP Semiconductors                                 */
/*=========================================================================*/

#ifndef _T8029_H_
#define _T8029_H_
#include "stm32f10x.h"  
#include "fifo.h"
//#define TDA1_RXBUF_SIZE   			272
//#define TDA_DC5_CNTL_Port 				GPIOC
//#define TDA_DC5_CNTL_Pin  				GPIO_PIN_5


#define TDA1_INT0_Port 				GPIOB
#define TDA1_INT0_Pin  				GPIO_Pin_0

#define TDA1_INT1_Port 				GPIOB
#define TDA1_INT1_Pin  				GPIO_Pin_1

//#define TDA1_P26_Port 				GPIOB
//#define TDA1_P26_Pin				GPIO_Pin_5

#define TDA1_RST_Port 				GPIOB
#define TDA1_RST_Pin				GPIO_Pin_8

// 
#define TDA1_SDWN_Port 				GPIOB
#define TDA1_SDWN_Pin				GPIO_Pin_9

//#define TDA1_WakeUpSlave_Port			GPIOB
//#define TDA1_WakeUpSlave_Pin			GPIO_PIN_1


//#define TDA1_SlaveI2CMute_Port		GPIOC
//#define TDA1_SlaveI2CMute_Pin		GPIO_PIN_8



 
//#define TDA1_WAKEUP_H	HAL_GPIO_WritePin(TDA1_WakeUpSlave_Port,TDA1_WakeUpSlave_Pin,GPIO_PIN_SET)
//#define TDA1_WAKEUP_L	HAL_GPIO_WritePin(TDA1_WakeUpSlave_Port,TDA1_WakeUpSlave_Pin,GPIO_PIN_RESET)

#define PORT_NUMBER						1



#define TDA8029_OK						0
#define TDA8029_ERROR					1
#define TDA8029_ALPARPATTERNNOK			2
#define TDA8029_ALPARPATTERNERROR		3
#define TDA8029_ALPARRESBADCMD			4
#define TDA8029_ALPARBADLENGTH			5
#define TDA8029_BUFFERTOOSMALL			6
#define TDA8029_COMMERROR				7
#define TDA8029_ALPARBADCHECKSUM		8

#define ALPAR_PATTERN_OK				0x60
#define ALPAR_PATTERN_NOK				0xE0

#define ALPAR_PATTERN_OFFSET			0
#define ALPAR_MSBLEN_OFFSET				1
#define ALPAR_LSBLEN_OFFSET				2
#define ALPAR_CMD_OFFSET				3
#define	ALPAR_DATA_OFFSET				4


#define ALPAR_CMD_MASK					0x0A
#define ALPAR_CMD_GETREADERSTATUS		0xAA
#define ALPAR_CMD_CHECKCARDPRES			0x09
#define ALPAR_CMD_POWERUP5V				0x6E
#define ALPAR_CMD_POWERUP3V				0x6D
#define ALPAR_CMD_POWERUP1V8			0x68
#define ALPAR_CMD_SENDAPDU				0x00

#define ALPARBUFSIZE					256
#define TDA_REC_BUFSIZE                 264
#define TDA1_UART_Handle  huart3 
#define TDA2_UART_Handle  huart4 

#define TDA_DEVICE1		0X01
#define TDA_DEVICE2		0X02
//typedef struct
//{
//	uint16_t wlen;
// 	uint16_t wPos;
// 	uint16_t rPos;
//	uint8_t rxBuf[ALPARBUFSIZE];
//	
//}DEV_Smart;
extern fifo_t tda1_fifo,tda2_fifo; 

unsigned char TDA8029_CheckPluggedDevice(void);
unsigned char TDA8029_CheckCardIn(void);
extern void TDA8029_TestStep(unsigned char testNum);
void TDA8029_IO_Config(void);
void TDA1_Usart3_Config(void);
unsigned char TDA8029_ALPAR_SendAndReceive (unsigned char cmd, unsigned char *pBufferReq, unsigned int BufferReqSize, unsigned char *receive_buffer, unsigned int *rlen);
unsigned char TDA8029_Wakeup(unsigned char *alparBuf, unsigned int  *rBufSize);
unsigned char TDA8029_CardPowerOn(void);

#endif

/***********************END OF FILE*********************************/
