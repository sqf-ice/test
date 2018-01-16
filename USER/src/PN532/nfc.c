#include "nfc.h"
#include "delay.h"
#include "string.h"
#include "stm32f10x_usart.h"
#include <stdio.h>
#include "pn532_dev.h"
#include "smartcard.h"
uint8_t PN532_RxBuffer[PN532_BUFFER_LEN];	//���ջ���
uint16_t PN532_RXCounter;	//���յ�����������
uint16_t PN532_rxnum;
  __IO u8 flag_rev_finish;		//���ڽ��յ����ݵı�־�� 1Ϊ���յ�


uint8_t UID[4];		   		//�洢 UID
uint8_t KeyBuf[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t BlockBuf[16];

void delay_10ms(uint16_t n10ms)
{
	uint32_t Delaytick;

	Delaytick = BspGetSysTicks();
	while(!BspGetDlyTicks(Delaytick,n10ms));

}
//��USART2������ length���ȵ�����data
void PN532_SendData(  uint8_t *data,uint8_t length)
{
    uint8_t i;
    for(i=0;i<length;i++)
    {
        USART_SendData(USART2, data[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);//�ȴ�������� 
    }
}

void CleanBuffer(uint16_t num)//���� ǰ ���ٸ��ֽڵ�����
{
    uint16_t i=0;
    
    for(i=0;i<num;i++)
	  PN532_RxBuffer[i]=0x00;
	PN532_RXCounter = 0;
}

/*********************************************************************************************************
��������  SAMConfiguration
�������ܣ�����PN532���ڲ�����
���������
	mode: defines the way of using the SAM
      	0x01:normal mode
      	0x02:virtual mode
      	0x03:wired mode
      	0x04:dual mode
 	timeout:
      	0x00:��ʹ��timeout
      	0x01-0xFF����ʱֵ
 	irq:
      	0x00����ʹ��P70_IRQ
      	0x01��ʹ��P70_IRQ
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
**********************************************************************************************************/
uint8_t PN532_SAMConfiguration(uint8_t mode,uint8_t timeout,uint8_t irq)
{
    uint8_t data[30];
    uint8_t i;
    uint8_t CheckCode=0; //����У����
    uint8_t temp=0;
//	uint8_t timeout = 10;
    while(1)
    { 
        flag_rev_finish=0;
        //PN532 wakeup
		data[0]=0x55;
        data[1]=0x55;
        data[2]=0x00;
        data[3]=0x00;
        data[4]=0x00;
        data[5]=0x00;
        data[6]=0x00;
        data[7]=0x00;
        data[8]=0x00;
        data[9]=0x00;
        data[10]=0x00;
        data[11]=0x00;
        data[12]=0x00;
        data[13]=0x00;
        
		data[14]=0x00;	 //֡ͷ
		data[15]=0x00;
        data[16]=0xFF;
       
	    data[17]=0x05; 			 //�� ����
        data[18]=0x100-data[17]; //�� ���� У��  0x100-data[3]
		
		data[19]=0xD4; //�����ʶ
		data[20]= SAMConfiguration; //�����ʶ��
		data[21]= mode;
		data[22]= timeout;
		data[23]= irq;

		temp = 	data[19]+ data[20] + data[21] + data[22] + data[23];
        data[24]=0x100 - temp; //���� У��   0x100-
        data[25]=0x00;
       #if EPOS_DEBUG
        printf("\r\nSAMConfiguration >>=== ");
		for(i= 0; i < 26;i++)
			printf("%02x ",data[i]);
		printf("\r\n");
		#endif
        PN532_SendData(data,26);//��USART2������ length���ȵ�����data
		delay_10ms(18);  
//		timeout = 10;
//		while(timeout--)
//		{
//			delay_10ms(10); 
//			if(flag_rev_finish)
//				break;
//			
//		}
		//00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00    
        if(flag_rev_finish)
        {         
            flag_rev_finish=0;
			temp = 0;
            for(i=11;i<13;i++)
            {
                temp+=PN532_RxBuffer[i];
            }
            CheckCode=0x100-temp;
            if(CheckCode==PN532_RxBuffer[13])
            {
				#if EPOS_DEBUG
				printf("<<===");
				for(i= 0; i < 13;i++)
					printf("%02x ",PN532_RxBuffer[i]);
				printf("\r\n");
				#endif
                CleanBuffer(40);//��� ���ڽ��ջ�����ǰ30 ���ֽ�����
                return 1; 
            }
			return 0;
        }
		return 0;
		
    }
}

/**************************************************************************************
��������  InListPassiveTarget
�������ܣ�PN532��⿨
���������
	maxTag:
            PN532����ʼ�����ĸ�����PN532һ��ֻ��ͬʱ����2�ſ�         
 	brty:
            0x00 : 106 kbps type A (ISO/IEC14443 Type A),
            0x01 : 212 kbps (FeliCa polling),
            0x02 : 424 kbps (FeliCa polling),
            0x03 : 106 kbps type B (ISO/IEC14443-3B),
            0x04 : 106 kbps Innovision Jewel tag.
	*CardID  
			��Ƭԭʼ���к�
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
uint8_t  nfc_InListPassiveTarget(uint8_t maxTag,uint8_t brty,uint8_t *CardID)
{
    uint8_t data[11];
    uint8_t i,timeout;
    uint8_t CheckCode=0; //����У����
    uint8_t temp=0;
	uint8_t data_len = 0;
    {   
        flag_rev_finish=0;
        data[0]=0x00;  //֡ͷ
        data[1]=0x00;  //��ʼ��־
        data[2]=0xFF;
        data[3]=0x04;  //������
        data[4]=0xFC;  //������ У��  0x100-data[3]
        
		data[5]=0xD4;  //�����ʶ��
        data[6]=0x4A;  //�����ʶ��
        data[7]=maxTag;
        data[8]=brty;
        
		data[9]=0x100 - data[5] - data[6] - data[7] - data[8];  //����У��
        data[10]=0x00;
        
        PN532_SendData(data,11);//��USART������ length���ȵ�����data
		#if EPOS_DEBUG
	    printf("InListPassiveTarget <<----");
		for(i= 0; i < 11;i++)
			printf("%02x ",data[i]);
		printf("\r\n");
		#endif  
		//delay_10ms(18); 
		timeout = 20;
		while(timeout--)
		{
			delay_10ms(1); 
			if(flag_rev_finish)
				break;
			
		}
        temp=0;
        if(flag_rev_finish)
        {      
// cpu �� 00 00 ff 00 ff 00 00 00 ff 1c e4 d5 4b 01 01 00 04 28 04 fd e6 08 99 10 78 80 90 02 20 90 00 00 00 00 00 fd e6 08 99 5c 00
//m1 ��   00 00 ff 00 ff 00 00 00 ff 0c f4 d5 4b 01 01 00 04 08 04 d0 bb e1 1a 48 00	
/***************
			                              D5 4B NbTg [ TargetData1 [ ] ] [ TargetData2 [ ] ]
			Tg SENS_RES11(2 bytes) SEL_RES(1 byte) NFCIDLength(1 byte) NFCID1[ ] (NFCIDLengthbytes) [ ATS[ ] ] (ATSLength bytes12)

			********/			
            flag_rev_finish=0;
			if((PN532_RxBuffer[12] == 0x4b) && (PN532_RxBuffer[13] != 0))
            {
				data_len = PN532_RxBuffer[9];
				
				for(i=11;i<(11+data_len);i++)
	            {
	                temp+=PN532_RxBuffer[i];
	            }
	            CheckCode=0x100-temp;
	            if(CheckCode==PN532_RxBuffer[11+data_len])
	            {
	                CardID[0]=PN532_RxBuffer[19];
	                CardID[1]=PN532_RxBuffer[20];
	                CardID[2]=PN532_RxBuffer[21];
	                CardID[3]=PN532_RxBuffer[22]; 
					//printf("\r\nUID is %02x %02x %02x %02x ",UID[0],UID[1],UID[2],UID[3]); 
					CleanBuffer(40);//��� ���ڽ��ջ�����ǰ30 ���ֽ����� 
	                return data_len;
	            }
			}
			CleanBuffer(40);
        }
		return 0;
		
    }
}
/**************************************************************************************
��������  nfc_InSelect
�������ܣ�PN532 ѡ��
���������
	Tag:
     
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
uint8_t  nfc_InSelect( uint8_t Tag)
{
    uint8_t data[11];
    uint8_t i,timeout;
    uint8_t CheckCode=0; //����У����
    uint8_t temp=0;
	uint8_t data_len = 0;
	
    {   
        flag_rev_finish=0;
        data[0]=0x00;  //֡ͷ
        data[1]=0x00;  //��ʼ��־
        data[2]=0xFF;
        data[3]=0x04;  //������
        data[4]=0xFC;  //������ У��  0x100-data[3]
        
		data[5]=0xD4;  //�����ʶ��
        data[6]=0x54;  //�����ʶ��
        data[7]=Tag;        
        
		data[8]=0x100 - data[5] - data[6] - data[7];  //����У��
        data[9]=0x00;
        
        PN532_SendData(data,10);//��USART������ length���ȵ�����data
		#if EPOS_DEBUG

	    printf("nfc_InSelect <<----");
		for(i= 0; i < 10;i++)
			printf("%02x ",data[i]);
		printf("\r\n");
		#endif
		
		//delay_10ms(18);  
		timeout = 20;
		while(timeout--)
		{
			delay_10ms(1); 
			if(flag_rev_finish)
				break;
			
		}
        temp=0;
        if(flag_rev_finish)
        {
            flag_rev_finish=0;
			if((PN532_RxBuffer[12] == 0x55) && (PN532_RxBuffer[13] == 0))
            {
				data_len = PN532_RxBuffer[9];				
				for(i=11;i<(11+data_len);i++)
	            {
	                temp+=PN532_RxBuffer[i];
	            }
	            CheckCode=0x100-temp;
	            if(CheckCode==PN532_RxBuffer[11+data_len])
	            {
//					printf("\r\n");
//					for(i= 0; i < data_len;i++)
//					printf("%02x ",PN532_RxBuffer[12+i]);
//					printf("\r\n");
					CleanBuffer(40);//��� ���ڽ��ջ�����ǰ30 ���ֽ����� 
	                return 1;
	            }
			}
			CleanBuffer(40);
        }
		return 0;
		
    }
}
//00 00 ff 08 f8 d4 40 01 c0 f1 00 01 08 31 00
uint8_t PN532_ReadCardID(void)
{
	uint8_t data[16];
    uint8_t i,data_len;
    uint8_t CheckCode=0; //����У����
    uint8_t temp=0 ;
	{
	    flag_rev_finish=0;
        data[0]=0x00;  //֡ͷ
        data[1]=0x00;  //��ʼ��־
        data[2]=0xFF;
        data[3]=0x08;  //������
        data[4]=(uint8_t)(0x100-data[3]);  //������ У��  0x100-data[3]
        
		data[5]=0xD4;  //�����ʶ��
        data[6]=0x40;  //�����ʶ��
        data[7]= 0x01;
        data[8]=0x00;
        
		data[9]=0xf1;  // 
        data[10]=0x00;
        data[11]=0x01;
        data[12]=0x08;
        data[13]=0x31;
        data[14]=0x00;
        
        PN532_SendData(data,15);//��USART������ length���ȵ�����data
		#if EPOS_DEBUG

	    printf("PN532_ReadCardID <<----");
		for(i= 0; i < 16;i++)
			printf("%02x ",data[i]);
		printf("\r\n");
		#endif
		delay_10ms(18); 
		 temp=0;
        if(flag_rev_finish)
        {         
            flag_rev_finish=0;
			if((PN532_RxBuffer[11] == 0xd5) && (PN532_RxBuffer[12] == 0x41))
            {
				data_len = PN532_RxBuffer[9];
				for(i=11;i<(11+data_len);i++)
	            {
	                temp+=PN532_RxBuffer[i];
	            }
	            CheckCode=0x100-temp;
	            if(CheckCode==PN532_RxBuffer[11+data_len])
	            {
					
					#if EPOS_DEBUG
					printf(">>===");
					for(i= 10; i < 12+data_len;i++)
						printf("%02x ",PN532_RxBuffer[i]);
					printf("\r\n");
					#endif

					CleanBuffer(40);//��� ���ڽ��ջ�����ǰ30 ���ֽ����� 
	                return 1;
	            }
			}
			CleanBuffer(40);
        }
	}
		return 0;
	
}
 

/**************************************************************************************
��������  nfc_PsdVerify
�������ܣ�Mifare ����֤,��ĳһ����֤ͨ���󶼿��Զ����������������еĿ���ж�д����
���������
    uint8_t AuthenticationType ��0x60 / 0x61 Authentication A / Authentication B
	uint8_t BlockNo��	0~63
	uint8_t * KeyData ��  ����Կ   
	uint8_t *SerialNum: �����к�	  
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
uint8_t  nfc_PsdVerify(uint8_t AuthenticationType,uint8_t BlockNo,uint8_t * KeyData,uint8_t *SerialNum)
{
    uint8_t data[22];
    uint8_t temp=0; 
    uint8_t i;
    uint8_t CheckCode=0; //����У����
    
    
    flag_rev_finish=0;
    data[0]=0x00;
    data[1]=0x00;
    data[2]=0xFF;
    
    data[3]=0x0F; //�� ����
    data[4]=0xF1; //�� ���� У��  0x100-data[3]
    
    data[5]=0xD4; //�����ʶ��
    data[6]=0x40; //�����ʶ��
    
    data[7]=0x01;
    data[8]=AuthenticationType;    //KeyA ��֤
    data[9]=BlockNo;  			   //���
    
    memcpy(&data[10],KeyData,6);   //��Կ
	memcpy(&data[16],SerialNum,4); //��Ƭԭʼ���к�
    for(i=5;i<20;i++)
    {
        temp+=data[i];
    }
    data[20]=0x100-temp;   
    data[21]=0x00;
    PN532_SendData(data,22);
     
    delay_10ms(25);

    temp=0;
    for(i=11;i<14;i++)
    {
        temp+=PN532_RxBuffer[i];
    }
    CheckCode=0x100-temp;
	//41 00
    if(CheckCode==PN532_RxBuffer[14])
    {
       if((PN532_RxBuffer[13]==0x00) && (PN532_RxBuffer[12]==0x41))	//��֤�ɹ�
       {
		 CleanBuffer(40);//��� ���ڽ��ջ�����ǰ40 ���ֽ�����
		 return 1;
       }
    }
	return 0;
}

/**************************************************************************************
��������  nfc_read
�������ܣ�Mifare ��������
���������
	uint8_t BlockNo��	   0~63
	uint8_t *BlockData ��  ������   
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
uint8_t  nfc_read(uint8_t BlockNO,uint8_t *ReadData)
{
    uint8_t data[12];
    uint8_t temp=0; 
    uint8_t i;
    uint8_t CheckCode=0; //����У����

    flag_rev_finish=0;
    data[0]=0x00;
    data[1]=0x00;
    data[2]=0xFF;
    
    data[3]=0x05; //�� ����
    data[4]=0xFB; //�� ���� У��  0x100-data[3]
    
    data[5]=0xD4; //�����ʶ��
    data[6]=0x40; //�����ʶ��
    
    data[7]=0x01;
    data[8]=0x30;
    data[9]=BlockNO; //���ڶ����16�ֽ����� 
    
    temp=0;
    for(i=5;i<10;i++)
    {
        temp+=data[i];
    }
	data[10]=0x100-temp; 
    data[11]=0x00;

    PN532_SendData(data,12);
    
    delay_10ms(25);
  
    temp=0;
    for(i=11;i<30;i++)
    {
        temp+=PN532_RxBuffer[i];
    }
    CheckCode=0x100-temp;
    if(CheckCode==PN532_RxBuffer[30])
    {              
        if((PN532_RxBuffer[13]==0x00) && (PN532_RxBuffer[12]==0x41))
		{
			memcpy(ReadData,&PN532_RxBuffer[14],16);
			CleanBuffer(40);
			return 1;
		}
    }
	CleanBuffer(40);
	return 0;
}
/**************************************************************************************
��������  nfc_write
�������ܣ�Mifare д������
���������
	uint8_t BlockNo��	   0~63
	uint8_t * BlockData ��  ������   
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
uint8_t  nfc_write(uint8_t BlockNO,uint8_t *WriteData)
{
    uint8_t data[28];
    uint8_t temp=0;
    uint8_t i;
    uint8_t CheckCode=0;
    
    data[0]=0x00;
    data[1]=0x00;
    data[2]=0xFF;
    
    data[3]=0x15; //�� ����
    data[4]=0xEB; //�� ���� У��  0x100-data[3]
    
    data[5]=0xD4; //�����ʶ��
    data[6]=0x40; //�����ʶ��
    
    data[7]=0x01; //��д����6�ֽ� ����1�����ֲ�
    data[8]=0xA0; //д
    data[9]=BlockNO; //д�ڶ����16�ֽ����� 
    
    memcpy(&data[10],WriteData,16);

    
    for(i=5;i<26;i++)
    {
        temp+=data[i];
    }
    data[26]=0x100-temp; 
    data[27]=0x00;

    PN532_SendData(data,28);
    
    delay_10ms(25);

    temp=0;
    for(i=11;i<14;i++)
    {
        temp+=PN532_RxBuffer[i];
    }
    CheckCode=0x100-temp;
    if(CheckCode==PN532_RxBuffer[14])
    {
         if((PN532_RxBuffer[13]==0x00) && (PN532_RxBuffer[12]==0x41))
         {
			 CleanBuffer(40);//��� ���ڽ��ջ�����ǰ40 ���ֽ�����
			 return 1;
         }
    }
	CleanBuffer(40);
	return 0;
}
int pn53x_build_frame(uint8_t *pbtFrame, uint16_t *pszFrame, const uint8_t *pbtData, const uint16_t szData)
{
 
	 uint8_t btDCS = (256 - 0xD4);
	uint16_t szPos;
  if (szData <= PN53x_NORMAL_FRAME__DATA_MAX_LEN) {
    // LEN - Packet length = data length (len) + checksum (1) + end of stream marker (1)
    pbtFrame[3] = szData + 1;
    // LCS - Packet length checksum
    pbtFrame[4] = 256 - (szData + 1);
    // TFI
    pbtFrame[5] = 0xD4;
    // DATA - Copy the PN53X command into the packet buffer
    memcpy(pbtFrame + 6, pbtData, szData);

    // DCS - Calculate data payload checksum
   
    for (  szPos = 0; szPos < szData; szPos++) 
	{
      btDCS -= pbtData[szPos];
    }
    pbtFrame[6 + szData] = btDCS;

    // 0x00 - End of stream marker
    pbtFrame[szData + 7] = 0x00;

    (*pszFrame) = szData + PN53x_NORMAL_FRAME__OVERHEAD;
  } else if (szData <= PN53x_EXTENDED_FRAME__DATA_MAX_LEN)
  {
    // Extended frame marker
    pbtFrame[3] = 0xff;
    pbtFrame[4] = 0xff;
    // LENm
    pbtFrame[5] = (szData + 1) >> 8;
    // LENl
    pbtFrame[6] = (szData + 1) & 0xff;
    // LCS
    pbtFrame[7] = 256 - ((pbtFrame[5] + pbtFrame[6]) & 0xff);
    // TFI
    pbtFrame[8] = 0xD4;
    // DATA - Copy the PN53X command into the packet buffer
    memcpy(pbtFrame + 9, pbtData, szData);

    // DCS - Calculate data payload checksum
  
    for (  szPos = 0; szPos < szData; szPos++) {
      btDCS -= pbtData[szPos];
    }
    pbtFrame[9 + szData] = btDCS;

    // 0x00 - End of stream marker
    pbtFrame[szData + 10] = 0x00;

    (*pszFrame) = szData + PN53x_EXTENDED_FRAME__OVERHEAD;
  } 
	else 
	{
//    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "We can't send more than %d bytes in a raw (requested: %" PRIdPTR ")", PN53x_EXTENDED_FRAME__DATA_MAX_LEN, szData);
		return NFC_ECHIP;
	}
	return NFC_SUCCESS;
}
//pn532_wakeup( )
int pn53x_transceive( const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout)
{
	uint8_t  abtFrame[PN532_BUFFER_LEN] = { 0x00, 0x00, 0xff };
	uint16_t szFrame = 0,temp;
	uint8_t data_len ,i,CheckCode;
	
	
	flag_rev_finish = 0;
	CleanBuffer(40);
	pn53x_build_frame(abtFrame, &szFrame, pbtTx, szTx);
	PN532_SendData(abtFrame,szFrame);//��USART������ length���ȵ�����data
	while(timeout--)
	{
		delay_10ms(2); 
		if(flag_rev_finish)
			break;
	}
	temp=0;
	if(flag_rev_finish)
    {
		flag_rev_finish = 0;
		data_len = PN532_RxBuffer[9];
		for(i=11;i<(11+data_len);i++)
		{
			temp+=PN532_RxBuffer[i];
		}
		CheckCode=0x100-temp;
		if(CheckCode==PN532_RxBuffer[11+data_len])
		{
			memcpy(pbtRx,&PN532_RxBuffer[11],data_len); 
			 CleanBuffer(40);
			return data_len;
		}
		else return -1;
	}
	else
	{
		 CleanBuffer(40);
		return -2;
	}
	
	return data_len;
	
	
}
 
int pn53x_initiator_transceive_bytes( const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx,
                                 const uint16_t szRx, int timeout)
{
  uint16_t  szExtraTxLen;
  uint8_t  abtCmd[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  uint8_t  abtRx[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  int res = 0;
  uint16_t szRxLen = PN53x_EXTENDED_FRAME__DATA_MAX_LEN;
  
 
    abtCmd[0] = InDataExchange;
    abtCmd[1] = 1;              /* target number */
    memcpy(abtCmd + 2, pbtTx, szTx);
    szExtraTxLen = 2;
//  } 
//	else {
//    abtCmd[0] = InCommunicateThru;
//    memcpy(abtCmd + 1, pbtTx, szTx);
//    szExtraTxLen = 1;
//  }
  // Send the frame to the PN53X chip and get the answer
  // We have to give the amount of bytes + (the two command bytes 0xD4, 0x42)
 
  if ((res = pn53x_transceive(abtCmd, szTx + szExtraTxLen, abtRx, sizeof(abtRx), timeout)) < 0) 
{
//    error 
	__nop();
	__nop();
	return res;
		
  
  }
  if(res > 3)
  {
	  szRxLen = (uint16_t)res - 3;
	  if (pbtRx != NULL) 
	  {    // Copy the received bytes
		  memcpy(pbtRx, abtRx + 3, szRxLen);
	  }
	  return szRxLen;
  }
  return res;  
  // Everything went successful, we return received bytes count
  
}

int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
{
  int res;
  size_t  szPos;
#if EPOS_DEBUG

  printf("=> ");
  for (szPos = 0; szPos < capdulen; szPos++) {
    printf("%02x ", capdu[szPos]);
  }
  printf("\n");
  #endif
  if ((res = pn53x_initiator_transceive_bytes( capdu, capdulen, rapdu, sizeof(SC_APDU_Resp_t) , 20)) < 0)  
  {
	  return -1;
  }
  else
  {
	  if(res < 2)
	  {
		  printf("err res is %02x \r\n",res);
		  return -2;
	  }
	  else
	  {
		  *rapdulen = (size_t) res;
		  #if EPOS_DEBUG
		  printf("<= ");
		  for (szPos = 0; szPos < *rapdulen; szPos++) 
		  {
			  printf("%02x ", rapdu[szPos]);
		  }
		  printf("\n");
		  #endif
		  return 0;
	  }
  }
}
int pn53x_pcd_transceive_bytes( const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx,
                                 const uint16_t szRx, int timeout)
{
  uint16_t  szExtraTxLen;
  uint8_t  abtCmd[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  uint8_t  abtRx[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  int res = 0;
  uint16_t szRxLen = PN53x_EXTENDED_FRAME__DATA_MAX_LEN;
   
 
     abtCmd[0] = InCommunicateThru;
     memcpy(abtCmd + 1, pbtTx, szTx);
     szExtraTxLen = 1;
 
  // Send the frame to the PN53X chip and get the answer
  // We have to give the amount of bytes + (the two command bytes 0xD4, 0x42)
 
  if ((res = pn53x_transceive(abtCmd, szTx + szExtraTxLen, abtRx, sizeof(abtRx), timeout)) < 0) 
{
//    error 
	__nop();
	__nop();
	return res;
		
  
  }
  if(res > 3)
  {
	  szRxLen = (uint16_t)res - 3;
	  if (pbtRx != NULL) 
	  {    // Copy the received bytes
		  memcpy(pbtRx, abtRx + 3, szRxLen);
	  }
	  return szRxLen;
  }
  return res;  
  // Everything went successful, we return received bytes count
  
}

int PCDTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
{
  int res;
  size_t  szPos;
	#if EPOS_DEBUG
  printf("=> ");
  for (szPos = 0; szPos < capdulen; szPos++) {
    printf("%02x ", capdu[szPos]);
  }
  printf("\n");
  	#endif

  if ((res = pn53x_pcd_transceive_bytes( capdu, capdulen, rapdu, sizeof(SC_APDU_Resp_t) , 500)) < 0)  
  {
	  return -1;
  }
  else
  {
	  if(res < 2)
	  {
		  printf("err res is %02x \r\n",res);
		  return -2;
	  }
	  else
	  {
		  *rapdulen = (size_t) res;
		  	#if EPOS_DEBUG

		  printf("<= ");
		  for (szPos = 0; szPos < *rapdulen; szPos++) 
		  {
			  printf("%02x ", rapdu[szPos]);
		  }
		  printf("\n");
		  #endif
		  return 0;
	  }
  }
}
/**************************************************************************************
��������  nfc_getIdentityID
�������ܣ���ȡ���֤UID
���������
	uint8_t *identityid  
���������
 	�ɹ���TRUE
 	ʧ�ܣ�FALSE
*****************************************************************************************/
#if 0
uint8_t 	nfc_getIdentityID(uint8_t *identityid)
{
   
}
#endif
