#ifndef __MSG_PROCESS_H
#define __MSG_PROCESS_H
#include "stm32f10x.h"

//ucCmd
#define GET_PURCHASE_PAY			0x10  //epos--> card-reader  消费余额  交易时间7 bytes + 余额 4 bytes
#define END_PURCHASE_PAY			0x11  //epos--> card-reader   取消交易  交易时间7 bytes + 余额 4 bytes

  
#define UPLOAD_DEVICE_STATUS		0x20
#define UPLOAD_REMAINING_MONEY		0x21  // card-reader --> epos  卡内余额
#define UPLOAD_PURCHASE_SUCCESS		0x22
#define UPLOAD_SOME_ERROR			0x23


#define MSG_MAXSIZE			256

#define RECV_PAYLOAD_SIZE 	64
#define RESP_PAYLOAD_SIZE 	(MSG_MAXSIZE-MSG_ALL_HEADSIZE-3)
#define HEAD_SIZE			8
#define MSG_HEADSIZE		8

#define MSG_ALL_HEADSIZE 	13

#define ANTIINTER_DATA  	0
#define SYNC_DATA	    	1
#define HEAD_DATA 			2
#define REL_DATA			3

#define MSG_DEAL_OK			0x00
#define MSG_HEADER_ERROR  	0x01
#define MSG_DATACRC_ERROR	0x02

typedef struct _MSG
{
	uint32_t wMsgType; 
	uint8_t Data[MSG_MAXSIZE];
}T_MSG;

typedef struct _MSG_HEADER
{ 
	const uint8_t AntiInterHead[2];
	const uint8_t SyncHead[3];
	uint8_t wDeviceID[2];
	uint8_t ucSerial;
	
	uint8_t ucFuc;
	uint8_t ucCmd;
	uint8_t wLen[2];
	
	uint8_t ucHeadSum; 
}T_MSG_HEADER;

typedef struct _MSG_RECV
{
	uint8_t wDeviceID[2];
	uint8_t ucSerial;
	uint8_t ucFuc;
	uint8_t ucCmd;
	uint8_t wLen[2];
	uint8_t ucHeadSum;
	uint8_t Data[RECV_PAYLOAD_SIZE];
	uint8_t wDataCrc[2];//
}T_MSG_RECV;

typedef struct _MSG_RESP
{ 
	T_MSG_HEADER header;
	uint8_t Data[RESP_PAYLOAD_SIZE];
	uint8_t wDataCrc[2];//
	uint8_t backup;
}T_MSG_RESP;
extern T_MSG g_tMsg;

uint8_t msg_pack(uint8_t cmd,uint8_t *data_buffer,uint8_t data_len ,T_MSG *t_msg );
int DealMsg(T_MSG *msg);
#endif