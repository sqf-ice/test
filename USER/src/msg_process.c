
#include "msg_process.h"
#include <stdio.h>
#include <string.h>
#include "encryption.h" 
#include "bsp_serial.h" 
#include "bsp_led.h"  

T_MSG g_tMsg;
extern uint8_t gDeviceID[2];
uint8_t msg_pack(uint8_t cmd,uint8_t *data_buffer,uint8_t data_len ,T_MSG *t_msg )
{
	uint8_t data[256],ucCheckSum,i;
	static uint8_t serial = 0;
	uint8_t CRCDATATemp[2];
	uint16_t t_msg_len;
	T_MSG_HEADER tHeader = {{0xaa,0xaa},{0xc0,0xc0,0xc0}};
	uint8_t *pCheck = (uint8_t*)tHeader.wDeviceID;
	//tHeader.AntiInterHead
	tHeader.wDeviceID[0] = gDeviceID[0];
	tHeader.wDeviceID[1] = gDeviceID[1];
	tHeader.ucSerial = serial++;
	tHeader.ucFuc = 0;
	tHeader.ucCmd = cmd;
	if(data_len >1)
	{
		tHeader.wLen[0] = (uint8_t)(data_len+2);
		tHeader.wLen[1] = (uint8_t)((data_len+2)>>8);
	}
	else
	{
		tHeader.wLen[0] = 0;
		tHeader.wLen[1] = 0;
	}

	ucCheckSum = 0;
	
	for(i=0;i<7;i++)
		ucCheckSum += *pCheck++;
	tHeader.ucHeadSum = ucCheckSum;
	

	 
	if((data_len >1) && (data_len< RESP_PAYLOAD_SIZE))
	{
		
		//计算crc
		if(data_buffer != NULL)
		{		
			CRC16(data_buffer,data_len,CRCDATATemp); 
			memcpy(&t_msg->Data[MSG_ALL_HEADSIZE],data_buffer,data_len);
			memcpy(&t_msg->Data[MSG_ALL_HEADSIZE+data_len],CRCDATATemp,2);			
		}
			
	}	
	memcpy(t_msg->Data,tHeader.AntiInterHead,MSG_ALL_HEADSIZE);
	t_msg_len = tHeader.wLen[0] +MSG_ALL_HEADSIZE;
	UART_Send_Data(t_msg->Data,t_msg_len);//UART_Put_Num(t_msg->Data,t_msg_len);
	
}
// 1: ok  0: fail
int8_t MsgHeaderCheck(T_MSG_RECV *msg_rec)
{
	uint8_t sum_head = 0,i;
	uint8_t *pData = (uint8_t*)msg_rec; 
	for(i=0;i<(MSG_HEADSIZE-1);i++)
	{
		sum_head = sum_head +*(pData++);
	}
	if(sum_head == msg_rec->ucHeadSum)
		return 1;
	else
		return 0;
	
	
}
// 1: ok  0: fail

int8_t MsgPackageCrcCheck(T_MSG_RECV *msg_rec)
{
	uint16_t wlen = 0;
	uint8_t crc_tmp[2];
	wlen = msg_rec->wLen[0] + (msg_rec->wLen[1] << 8); 
	if(wlen > 2)
	{
		CRC16(msg_rec->Data,wlen-2,crc_tmp);
		if((crc_tmp[0] == msg_rec->Data[wlen-2]) && (crc_tmp[1] == msg_rec->Data[wlen-1]))
			return 1;
		else
			return 0;
		
	}
	else
		return 1;
		
	
}
extern unsigned char  gPurchase_Pay[4];
extern unsigned char  gRecDateTime[7];
extern unsigned char  gPurchaseFlag;
int DealMsg(T_MSG *msg)
{
	//uint8_t i;
 
	T_MSG_RECV *ptRecvMsg; 
	T_MSG_RESP tRespMsg={{0xaa,0xaa},{0xc0,0xc0,0xc0}}; 
//	ParameterStruct tGetPar;
//	DateTime tTime;
//	GetSetedPar(&tGetPar);
	ptRecvMsg = (T_MSG_RECV *)&msg->Data[5];
	
	//DateTime *start;
//	DateTime *end;
	//ptRespMsg = (T_MSG_RESP *)&msg->Data[0];
 
 //CRC 校验
	if(MsgHeaderCheck(ptRecvMsg) != 1)
	{
		return MSG_HEADER_ERROR;
	}
	if(MsgPackageCrcCheck(ptRecvMsg) != 1)
	{
		return MSG_DATACRC_ERROR;
	}	
	switch(ptRecvMsg->ucCmd)
	{
// 		case 0:  /*for test*/
		case 0:
			LED2_TOGGLE;
			break;
		case GET_PURCHASE_PAY:
			//
            memcpy(gRecDateTime,ptRecvMsg->Data,7);
			memcpy(gPurchase_Pay,&ptRecvMsg->Data[7],4);
			gPurchaseFlag = 1;

			break;
		case END_PURCHASE_PAY:  
			memset(gPurchase_Pay,0, 4);
			memcpy(gRecDateTime,0x00,7);
    		gPurchaseFlag = 0;
			break;  
	 
		default: 
			//tRespMsg.ucCmd = 0xff;
			break;
	}	
	return MSG_DEAL_OK; 
}
	



#if 0

T_MSG gMsgPro;
 // extern QueueHandle_t g_pDataProcQueue;

int MsgRSEPPackage(T_MSG_RECV *ptdev,uint8_t type)//设置指令回复
{ 
	uint16_t wlen; 	 
	//uint8_t checksum;
	T_MSG tMsg;
	T_MSG_RESP *tRespMsg = (T_MSG_RESP *)&tMsg.Data[0];
//	uint8_t *pCheck = tRespMsg->wDeviceID;
	 
	//wLength = ptdev->wLen[0] + (ptdev->wLen[1]<<8); 	
//	memset(tRespMsg->AntiInterHead, 0xaa, 2); 
//	memset(tRespMsg->SyncHead, 0xc0, 3); 
//	tRespMsg.ucSerial  = ptdev->ucSerial;
//	tRespMsg.wDeviceID[0] = ptdev->wDeviceID[0];	 
//	tRespMsg.wDeviceID[1] = ptdev->wDeviceID[1];	 	
//	tRespMsg.ucCmd    =   ptdev->ucCmd;
//	tRespMsg.wLen[0]  =  ptdev->wLen[0];//
//	tRespMsg.wLen[1]  =  ptdev->wLen[1];
//	tRespMsg.ucHeadSum = ptdev->ucHeadSum;
	memcpy(tRespMsg->wDeviceID,ptdev->wDeviceID,8);	
	wlen = tRespMsg->wLen[0] + (tRespMsg->wLen[1]<<8);
	if(wlen > 2)
	{
		memcpy(tRespMsg->Data,ptdev->Data,wlen-2);
		tRespMsg->Data[wlen-2] = ptdev->wDataCrc[0];
		tRespMsg->Data[wlen-1] = ptdev->wDataCrc[1];
		tRespMsg->wDataCrc[0] = ptdev->wDataCrc[0];
		tRespMsg->wDataCrc[1] = ptdev->wDataCrc[1];
	}
	else
	{
		__nop();
	}
	tMsg.wMsgType = type;
	memcpy((uint8_t*)&gMsgPro,(uint8_t*)&tMsg,sizeof(T_MSG));
	return 0;
} 

/*****************
***   for  test
***
*****************/

//int MsgPackage(T_MSG_RECV *ptdev)
int MsgPackage(T_MSG_RESP *ptRespMsg,const uint8_t type ) 
{ 
	//struct GPRS_DEV *pDev = GetGprsDev();
	
	uint8_t	ucCheckSum;
	static uint8_t ucSerNum = 0;
	
	uint8_t* pCheck = (uint8_t*)&ptRespMsg->wDeviceID;
 
	uint8_t CRCDATATemp[2];
	
	uint16_t i,wLen; 	
	T_MSG tMsg;
 
	ucSerNum++;
 
//	memset(ptRespMsg->AntiInterHead, 0xaa, 2); 
//	memset(ptRespMsg->SyncHead, 0xc0, 3); 
	ptRespMsg->wDeviceID[0] = 1;//tGetPar.DeviceID[0];			
	ptRespMsg->wDeviceID[1] = 0;//tGetPar.DeviceID[1]; 	
	ptRespMsg->ucSerial = ucSerNum;
	ptRespMsg->ucFuc = 0;
	ucCheckSum = 0;
	for(i=0;i<7;i++)
		ucCheckSum += *pCheck++;
	ptRespMsg->ucHeadSum = ucCheckSum;
	wLen = ptRespMsg->wLen[0] + (ptRespMsg->wLen[1]<<8);
 
	if(wLen>2)
	{
		if((MSG_ALL_HEADSIZE+wLen)> MSG_MAXSIZE)
			return -1;
		CRC16(ptRespMsg->Data,wLen-2,CRCDATATemp); 
		ptRespMsg->wDataCrc[0]= CRCDATATemp[0];
		ptRespMsg->wDataCrc[1]= CRCDATATemp[1];
		ptRespMsg->Data[wLen-2] = CRCDATATemp[0];
		ptRespMsg->Data[wLen-1] = CRCDATATemp[1];
		memset((uint8_t*)&tMsg,0,sizeof(tMsg));
		memcpy(tMsg.Data,ptRespMsg->AntiInterHead,MSG_ALL_HEADSIZE+wLen);
		
	}
	else
	{
		memcpy(tMsg.Data,ptRespMsg->AntiInterHead,MSG_ALL_HEADSIZE);
		
	}
	tMsg.wMsgType = type;	
	memcpy((uint8_t*)&gMsgPro,(uint8_t*)&tMsg,sizeof(T_MSG));
//	if(xQueueSend(g_pDataProcQueue, &tMsg, 0) == pdPASS)
//	{
//		__nop();
//	}
	return 0;
} 
/******
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
****/


//Uart3_RecvHandler 接收有效数据填充至 T_MSG tMsg
//DealMsg 将T_MSG中DATA 给	T_MSG_RECV *ptRecvMsg; 
int DealMsg(T_MSG *msg)
{
	//uint8_t i;
 
	T_MSG_RECV *ptRecvMsg; 
	T_MSG_RESP tRespMsg={{0xaa,0xaa},{0xc0,0xc0,0xc0}}; 
//	ParameterStruct tGetPar;
//	DateTime tTime;
//	GetSetedPar(&tGetPar);
	ptRecvMsg = (T_MSG_RECV *)&msg->Data[5];
	
	//DateTime *start;
//	DateTime *end;
	//ptRespMsg = (T_MSG_RESP *)&msg->Data[0];
 
 //CRC 校验
	if(MsgHeaderCheck(ptRecvMsg) != 1)
	{
		return MSG_HEADER_ERROR;
	}
	if(MsgPackageCrcCheck(ptRecvMsg) != 1)
	{
		return MSG_DATACRC_ERROR;
	}
	
	switch(ptRecvMsg->ucCmd)
	{
// 		case 0:  /*for test*/
		case 1:
			break;
	 
		default: 
			tRespMsg.ucCmd = 0xff;
			break;
	}
 
	
	return MSG_DEAL_OK; 
}

#endif