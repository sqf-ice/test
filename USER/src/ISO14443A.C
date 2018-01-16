//#include "Rc531.H"
#include "iso14443a.h"
#include "stm32f10x.h"
#include "mirror-subr.h"
#include "pn532_dev.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "nfc.h"
 extern __IO u8 flag_rev_finish;
 #define MAX_FRAME_LEN 264

  static uint8_t abtRx[MAX_FRAME_LEN];
static int szRxBits;
static size_t szRx = sizeof(abtRx);
static uint8_t abtRawUid[12];
static uint8_t abtAtqa[2];
static uint8_t abtSak;
static uint8_t abtAts[MAX_FRAME_LEN];
static uint8_t szAts = 0;
static size_t szCL = 1;//Always start with Cascade Level 1 (CL1)
// int pn53x_transceive( const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout)
//{
//	uint8_t  abtFrame[PN532_BUFFER_LEN] = { 0x00, 0x00, 0xff };
//	uint16_t szFrame = 0,temp;
//	uint8_t data_len ,i,CheckCode;
//	
//	
//	flag_rev_finish = 0;
//	CleanBuffer(40);
//	pn53x_build_frame(abtFrame, &szFrame, pbtTx, szTx);
//	SendData(abtFrame,szFrame);//往USART，发送 length长度的数据data
//	delay_10ms(25); 
//	temp=0;
//	if(flag_rev_finish)
//    {
//		flag_rev_finish = 0;
//		data_len = PN532_RxBuffer[9];
//		for(i=11;i<(11+data_len);i++)
//		{
//			temp+=PN532_RxBuffer[i];
//		}
//		CheckCode=0x100-temp;
//		if(CheckCode==PN532_RxBuffer[11+data_len])
//		{
//			memcpy(pbtRx,&PN532_RxBuffer[11],data_len); 
//			 CleanBuffer(40);
//			return data_len;
//		}
//		else return -1;
//	}
//	else
//	{
//		 CleanBuffer(40);
//		return -2;
//	}
//	
//	return data_len;
//	
//	
//}
 int last_error,last_command,last_status_byte;

int pn53x_transceive_new(   const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout)
{
  bool mi = false;
  int res = 0;
//  if (CHIP_DATA(pnd)->wb_trigged) {
//    if ((res = pn53x_writeback_register(pnd)) < 0) {
//      return res;
//    }
//  }
 

  uint8_t  abtRx[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  size_t  szRx = sizeof(abtRx);

  // Check if receiving buffers are available, if not, replace them
  if (szRxLen == 0 || !pbtRx) {
    pbtRx = abtRx;
  } else {
    szRx = szRxLen;
  }

  // Call the send/receice callback functions of the current driver
    PN532_SendData((uint8_t*)pbtTx, szTx); 
  
	delay_10ms(25); 
  // Command is sent, we store the command
  last_command = pbtTx[0];

  // Handle power mode for PN532
//  if ((CHIP_DATA(pnd)->type == PN532) && (TgInitAsTarget == pbtTx[0])) {  // PN532 automatically goes into PowerDown mode when TgInitAsTarget command will be sent
//    CHIP_DATA(pnd)->power_mode = POWERDOWN;
//  }
	if(flag_rev_finish)
	{
     //pbtRx, szRx, 
	}

//  if ((CHIP_DATA(pnd)->type == PN532) && (TgInitAsTarget == pbtTx[0])) { // PN532 automatically wakeup on external RF field
//     power_mode = NORMAL; // When TgInitAsTarget reply that means an external RF have waken up the chip
//  }

  switch (pbtTx[0]) {
    case PowerDown:
    case InDataExchange:
    case InCommunicateThru:
    case InJumpForPSL:
    case InPSL:
    case InATR:
    case InSelect:
    case InJumpForDEP:
    case TgGetData:
    case TgGetInitiatorCommand:
    case TgSetData:
    case TgResponseToInitiator:
    case TgSetGeneralBytes:
    case TgSetMetaData:
      //if (pbtRx[0] & 0x80) { abort(); } // NAD detected
//      if (pbtRx[0] & 0x40) { abort(); } // MI detected
      mi = pbtRx[0] & 0x40;
       last_status_byte = pbtRx[0] & 0x3f;
      break;
    case Diagnose:
      if (pbtTx[1] == 0x06) { // Diagnose: Card presence detection
         last_status_byte = pbtRx[0] & 0x3f;
      } else {
        last_status_byte = 0;
      };
      break;
    case InDeselect:
    case InRelease:
      
       last_status_byte = pbtRx[0] & 0x3f;
      break;
    case ReadRegister:
    case WriteRegister:
   
        last_status_byte = 0;
      
      break;
    default:
       last_status_byte = 0;
  }

  while (mi) {
    int res2;
    uint8_t  abtRx2[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
    // Send empty command to card
   PN532_SendData((uint8_t*)pbtTx, 2);

   if(flag_rev_finish)
   {
		 mi = abtRx2[0] & 0x40;
		if ((size_t)(res + res2 - 1) > szRx) {
		   last_status_byte = ESMALLBUF;
		  break;
		}
		memcpy(pbtRx + res, abtRx2 + 1, res2 - 1);
		// Copy last status byte
		pbtRx[0] = abtRx2[0];
		res += res2 - 1;
	}
  }

  szRx = (size_t) res;

  switch (  last_status_byte) {
    case 0:
      res = (int)szRx;
      break;
    case ETIMEOUT:
    case ECRC:
    case EPARITY:
    case EBITCOUNT:
    case EFRAMING:
    case EBITCOLL:
    case ERFPROTO:
    case ERFTIMEOUT:
    case EDEPUNKCMD:
    case EDEPINVSTATE:
    case ENAD:
    case ENFCID3:
    case EINVRXFRAM:
    case EBCC:
    case ECID:
      res = NFC_ERFTRANS;
      break;
    case ESMALLBUF:
    case EOVCURRENT:
    case EBUFOVF:
    case EOVHEAT:
    case EINBUFOVF:
      res = NFC_ECHIP;
      break;
    case EINVPARAM:
    case EOPNOTALL:
    case ECMD:
    case ENSECNOTSUPP:
      res = NFC_EINVARG;
      break;
    case ETGREL:
    case ECDISCARDED:
      res = NFC_ETGRELEASED;
      //pn53x_current_target_free(pnd);
      break;
    case EMFAUTH:
      // When a MIFARE Classic AUTH fails, the tag is automatically in HALT state
      res = NFC_EMFCAUTHFAIL;
      break;
    default:
      res = NFC_ECHIP;
      break;
  };

  if (res < 0) {
     last_error = res;
  } else {
     last_error = 0;
  }
  return res;
}
 static int pn53x_ReadRegister( uint16_t ui16RegisterAddress, uint8_t *ui8Value)
{
  uint8_t  abtCmd[3] = {0 };
  uint8_t  abtRegValue[2];
  size_t  szRegValue = sizeof(abtRegValue);
  int res = 0;
	abtCmd[0] = ReadRegister;
	abtCmd[1] = ui16RegisterAddress >> 8;
	abtCmd[2] = ui16RegisterAddress & 0xff ;

  if ((res = pn53x_transceive_new( abtCmd, sizeof(abtCmd), abtRegValue, szRegValue, -1)) < 0) {
    return res;
  }
//  if (CHIP_DATA(pnd)->type == PN533) {
//    // PN533 prepends its answer by a status byte
//    *ui8Value = abtRegValue[1];
//  } else {
    *ui8Value = abtRegValue[0];
  //}
  return NFC_SUCCESS;
}
static int pn53x_WriteRegister(  const uint16_t ui16RegisterAddress, const uint8_t ui8Value)
{
  uint8_t  abtCmd[4] = {0};
	abtCmd[0] = WriteRegister;
	abtCmd[1] = ui16RegisterAddress >> 8 ;
	abtCmd[2] =   ui16RegisterAddress & 0xff ;
	abtCmd[3] = ui8Value ;
  
  return pn53x_transceive_new(abtCmd, sizeof(abtCmd), NULL, 0, -1);
}
 
int pn53x_write_register(  const uint16_t ui16RegisterAddress, const uint8_t ui8SymbolMask, const uint8_t ui8Value)
{
	uint8_t ui8NewValue =0;
	if ((ui16RegisterAddress < PN53X_CACHE_REGISTER_MIN_ADDRESS) || (ui16RegisterAddress > PN53X_CACHE_REGISTER_MAX_ADDRESS)) 
	{
		// Direct write
		if (ui8SymbolMask != 0xff) 
		{
			int res = 0;
			uint8_t ui8CurrentValue;
			if ((res = pn53x_ReadRegister(  ui16RegisterAddress, &ui8CurrentValue)) < 0)
				return res;
			  ui8NewValue = ((ui8Value & ui8SymbolMask) | (ui8CurrentValue & (~ui8SymbolMask)));
			if (ui8NewValue != ui8CurrentValue) 
			{
				return pn53x_WriteRegister(  ui16RegisterAddress, ui8NewValue); 
			}
		}
		else
		{
			return pn53x_WriteRegister(  ui16RegisterAddress, ui8Value);
		}
	}
	return NFC_SUCCESS;
}

 
unsigned char ui8TxBits;
int pn53x_set_tx_bits(  const uint8_t ui8Bits)
{
  // Test if we need to update the transmission bits register setting
  if ( ui8TxBits != ui8Bits) {
    int res = 0;
    // Set the amount of transmission bits in the PN53X chip register
    if ((res = pn53x_write_register( PN53X_REG_CIU_BitFraming, SYMBOL_TX_LAST_BITS, ui8Bits)) < 0)
      return res;

    // Store the new setting
     ui8TxBits = ui8Bits;
  }
  return NFC_SUCCESS;
}

int pn53x_wrap_frame(const uint8_t *pbtTx, const size_t szTxBits, const uint8_t *pbtTxPar,uint8_t *pbtFrame)
{
  uint8_t  btData;
  uint32_t uiBitPos;
  uint32_t uiDataPos = 0;
  size_t  szBitsLeft = szTxBits;
  size_t szFrameBits = 0;

  // Make sure we should frame at least something
  if (szBitsLeft == 0)
    return NFC_ECHIP;

  // Handle a short response (1byte) as a special case
  if (szBitsLeft < 9) {
    *pbtFrame = *pbtTx;
    szFrameBits = szTxBits;
    return szFrameBits;
  }
  // We start by calculating the frame length in bits
  szFrameBits = szTxBits + (szTxBits / 8);

  // Parse the data bytes and add the parity bits
  // This is really a sensitive process, mirror the frame bytes and append parity bits
  // buffer = mirror(frame-byte) + parity + mirror(frame-byte) + parity + ...
  // split "buffer" up in segments of 8 bits again and mirror them
  // air-bytes = mirror(buffer-byte) + mirror(buffer-byte) + mirror(buffer-byte) + ..
  while (true) {
    // Reset the temporary frame byte;
    uint8_t  btFrame = 0;

    for (uiBitPos = 0; uiBitPos < 8; uiBitPos++) {
      // Copy as much data that fits in the frame byte
      btData = mirror(pbtTx[uiDataPos]);
      btFrame |= (btData >> uiBitPos);
      // Save this frame byte
      *pbtFrame = mirror(btFrame);
      // Set the remaining bits of the date in the new frame byte and append the parity bit
      btFrame = (btData << (8 - uiBitPos));
      btFrame |= ((pbtTxPar[uiDataPos] & 0x01) << (7 - uiBitPos));
      // Backup the frame bits we have so far
      pbtFrame++;
      *pbtFrame = mirror(btFrame);
      // Increase the data (without parity bit) position
      uiDataPos++;
      // Test if we are done
      if (szBitsLeft < 9)
        return szFrameBits;
      szBitsLeft -= 8;
    }
    // Every 8 data bytes we lose one frame byte to the parities
    pbtFrame++;
  }
}

int pn53x_unwrap_frame(const uint8_t *pbtFrame, const size_t szFrameBits, uint8_t *pbtRx, uint8_t *pbtRxPar)
{
  uint8_t  btFrame;
  uint8_t  btData;
  uint8_t uiBitPos;
  uint32_t uiDataPos = 0;
  uint8_t *pbtFramePos = (uint8_t *) pbtFrame;
  size_t  szBitsLeft = szFrameBits;
  size_t szRxBits = 0;

  // Make sure we should frame at least something
  if (szBitsLeft == 0)
    return NFC_ECHIP;

  // Handle a short response (1byte) as a special case
  if (szBitsLeft < 9) {
    *pbtRx = *pbtFrame;
    szRxBits = szFrameBits;
    return szRxBits;
  }
  // Calculate the data length in bits
  szRxBits = szFrameBits - (szFrameBits / 9);

  // Parse the frame bytes, remove the parity bits and store them in the parity array
  // This process is the reverse of WrapFrame(), look there for more info
  while (true) {
    for (uiBitPos = 0; uiBitPos < 8; uiBitPos++) {
      btFrame = mirror(pbtFramePos[uiDataPos]);
      btData = (btFrame << uiBitPos);
      btFrame = mirror(pbtFramePos[uiDataPos + 1]);
      btData |= (btFrame >> (8 - uiBitPos));
      pbtRx[uiDataPos] = mirror(btData);
      if (pbtRxPar != NULL)
        pbtRxPar[uiDataPos] = ((btFrame >> (7 - uiBitPos)) & 0x01);
      // Increase the data (without parity bit) position
      uiDataPos++;
      // Test if we are done
      if (szBitsLeft < 9)
        return szRxBits;
      szBitsLeft -= 9;
    }
    // Every 8 data bytes we lose one frame byte to the parities
    pbtFramePos++;
  }
}
 
 
 

int pn53x_initiator_transceive_bits( const uint8_t *pbtTx, const size_t szTxBits,
                                const uint8_t *pbtTxPar, uint8_t *pbtRx, uint8_t *pbtRxPar)
{
  int res = 0;
  size_t  szFrameBits = 0;
  size_t  szFrameBytes = 0;
  size_t szRxBits = 0;
  uint8_t ui8rcc;
  uint8_t ui8Bits = 0;
  uint8_t  abtCmd[PN53x_EXTENDED_FRAME__DATA_MAX_LEN] = { InCommunicateThru };
  uint8_t  abtRx[PN53x_EXTENDED_FRAME__DATA_MAX_LEN];
  size_t  szRx = 0;
  szRx = sizeof(abtRx);
  // Check if we should prepare the parity bits ourself
  if (szTxBits > 0)
  {
	  // Convert data with parity to a frame
	  if ((res = pn53x_wrap_frame(pbtTx, szTxBits, pbtTxPar, abtCmd + 1)) < 0)
		  return res;
	  szFrameBits = res;
  }  

  // Retrieve the leading bits
  ui8Bits = szFrameBits % 8;

  // Get the amount of frame bytes + optional (1 byte if there are leading bits)
  szFrameBytes = (szFrameBits / 8) + ((ui8Bits == 0) ? 0 : 1);

  // When the parity is handled before us, we just copy the data
//  if (pnd->bPar)
//    memcpy(abtCmd + 1, pbtTx, szFrameBytes);

  // Set the amount of transmission bits in the PN53X chip register
  if ((res = pn53x_set_tx_bits( ui8Bits)) < 0)
    return res;

  // Send the frame to the PN53X chip and get the answer
  // We have to give the amount of bytes + (the command byte 0x42)
  
  if ((res = pn53x_transceive_new( abtCmd, szFrameBytes + 1, abtRx, szRx, -1)) < 0)
    return res;
  szRx = (size_t) res;
  // Get the last bit-count that is stored in the received byte
  if ((res = pn53x_ReadRegister(PN53X_REG_CIU_Control, &ui8rcc)) < 0)
    return res;
  ui8Bits = ui8rcc & SYMBOL_RX_LAST_BITS;

  // Recover the real frame length in bits
  szFrameBits = ((szRx - 1 - ((ui8Bits == 0) ? 0 : 1)) * 8) + ui8Bits;

  if (pbtRx != NULL) {
    // Ignore the status byte from the PN53X here, it was checked earlier in pn53x_transceive()
    // Check if we should recover the parity bits ourself
//    if (!pnd->bPar) {
//      // Unwrap the response frame
//      if ((res = pn53x_unwrap_frame(abtRx + 1, szFrameBits, pbtRx, pbtRxPar)) < 0)
//        return res;
//      szRxBits = res;
//    } else {
//      // Save the received bits
//      szRxBits = szFrameBits;
//      // Copy the received bytes
//      memcpy(pbtRx, abtRx + 1, szRx - 1);
//    }
  }
  // Everything went successful
  return szRxBits;
}
 int nfc_initiator_transceive_bits(const uint8_t *pbtTx, const size_t szTxBits, const uint8_t *pbtTxPar,
                              uint8_t *pbtRx, const size_t szRx,
                              uint8_t *pbtRxPar)
{
  (void)szRx;
   return pn53x_initiator_transceive_bits( pbtTx, szTxBits, pbtTxPar, pbtRx, pbtRxPar);
	
}

static  bool transmit_bits(const uint8_t *pbtTx, const size_t szTxBits)
{
  uint32_t cycles = 0;
  // Show transmitted command
//  if (!quiet_output) {
//    printf("Sent bits:     ");
//    print_hex_bits(pbtTx, szTxBits);
//  }
  // Transmit the bit frame command, we don't use the arbitrary parity feature
   if ((szRxBits = nfc_initiator_transceive_bits( pbtTx, szTxBits, NULL, abtRx, sizeof(abtRx),NULL)) < 0)
      return false;
 
  // Show received answer
//  if (!quiet_output) {
//    printf("Received bits: ");
//    print_hex_bits(abtRx, szRxBits);
//  }
  // Succesful transfer
  return true;

} 

int pn53x_set_property_bool( const nfc_property property, const bool bEnable)
{
  uint8_t  btValue;
  int res = 0;
  switch (property) {
    case NP_HANDLE_CRC:
      // Enable or disable automatic receiving/sending of CRC bytes
       // TX and RX are both represented by the symbol 0x80
      btValue = (bEnable) ? 0x80 : 0x00;
      if ((res = pn53x_write_register( PN53X_REG_CIU_TxMode, SYMBOL_TX_CRC_ENABLE, btValue)) < 0)
        return res;
      if ((res = pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_CRC_ENABLE, btValue)) < 0)
        return res;      
      return NFC_SUCCESS;

    case NP_HANDLE_PARITY:
      // Handle parity bit by PN53X chip or parse it as data bit
  
      btValue = (bEnable) ? 0x00 : SYMBOL_PARITY_DISABLE;
      if ((res = pn53x_write_register( PN53X_REG_CIU_ManualRCV, SYMBOL_PARITY_DISABLE, btValue)) < 0)
        return res;
      
      return NFC_SUCCESS; 
    case NP_ACTIVATE_FIELD:
     // return pn53x_RFConfiguration__RF_field( bEnable);
		break;
    case NP_ACTIVATE_CRYPTO1:
      btValue = (bEnable) ? SYMBOL_MF_CRYPTO1_ON : 0x00;
      return pn53x_write_register( PN53X_REG_CIU_Status2, SYMBOL_MF_CRYPTO1_ON, btValue);

    case NP_INFINITE_SELECT:
      // TODO Made some research around this point:
      // timings could be tweak better than this, and maybe we can tweak timings
      // to "gain" a sort-of hardware polling (ie. like PN532 does)
//      
//      return pn53x_RFConfiguration__MaxRetries(
//                                               (bEnable) ? 0xff : 0x00,        // MxRtyATR, default: active = 0xff, passive = 0x02
//                                               (bEnable) ? 0xff : 0x01,        // MxRtyPSL, default: 0x01
//                                               (bEnable) ? 0xff : 0x02         // MxRtyPassiveActivation, default: 0xff (0x00 leads to problems with PN531)
//                                              );

    case NP_ACCEPT_INVALID_FRAMES:
      btValue = (bEnable) ? SYMBOL_RX_NO_ERROR : 0x00;
      return pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_NO_ERROR, btValue);

    case NP_ACCEPT_MULTIPLE_FRAMES:
      btValue = (bEnable) ? SYMBOL_RX_MULTIPLE : 0x00;
      return pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_MULTIPLE, btValue);

    case NP_AUTO_ISO14443_4:
     
//      pnd->bAutoIso14443_4 = bEnable;
//      return pn53x_set_parameters( PARAM_AUTO_RATS, bEnable);

    case NP_FORCE_ISO14443_A:
      if (!bEnable) {
        // Nothing to do
        return NFC_SUCCESS;
      }
      // Force pn53x to be in ISO14443-A mode
      if ((res = pn53x_write_register( PN53X_REG_CIU_TxMode, SYMBOL_TX_FRAMING, 0x00)) < 0) {
        return res;
      }
      if ((res = pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_FRAMING, 0x00)) < 0) {
        return res;
      }
      // Set the PN53X to force 100% ASK Modified miller decoding (default for 14443A cards)
      return pn53x_write_register( PN53X_REG_CIU_TxAuto, SYMBOL_FORCE_100_ASK, 0x40);

    case NP_FORCE_ISO14443_B:
      if (!bEnable) {
        // Nothing to do
        return NFC_SUCCESS;
      }
      // Force pn53x to be in ISO14443-B mode
      if ((res = pn53x_write_register( PN53X_REG_CIU_TxMode, SYMBOL_TX_FRAMING, 0x03)) < 0) {
        return res;
      }
      return pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_FRAMING, 0x03);

    case NP_FORCE_SPEED_106:
      if (!bEnable) {
        // Nothing to do
        return NFC_SUCCESS;
      }
      // Force pn53x to be at 106 kbps
      if ((res = pn53x_write_register( PN53X_REG_CIU_TxMode, SYMBOL_TX_SPEED, 0x00)) < 0) {
        return res;
      }
      return pn53x_write_register( PN53X_REG_CIU_RxMode, SYMBOL_RX_SPEED, 0x00);
    // Following properties are invalid (not boolean)
    default:
		break;
  }

  return NFC_EINVARG;
}

void iso_or_config(void)
{
	
  // Configure the CRC
  if (pn53x_set_property_bool( NP_HANDLE_CRC, false) < 0) {
  
  }
  // Use raw send/receive methods
  if (pn53x_set_property_bool( NP_EASY_FRAMING, false) < 0) {
  
  }
  // Disable 14443-4 autoswitching
  if (pn53x_set_property_bool( NP_AUTO_ISO14443_4, false) < 0) {
 
  }
}


extern int PCDTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen);
/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//         pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro
//                0x0403 = Mifare_ProX
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
		char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 	uint16_t i = 0;	 
	uint8_t senddata[16];
	uint16_t rxLen;
	uint8_t rxbuf[64];
 
	senddata[0] = req_code;
 	if(PCDTransmit(senddata,1,rxbuf,&rxLen) == 0)
	{ 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		 if(rxLen >= 2)
		 {
			*pTagType     = rxbuf[0];
			*(pTagType+1) = rxbuf[1];
			 return 0;
		 }			 
  
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////
//防冲撞
//input: g_cSNR=存放序列号(4byte)的内存单元首地址
//output:status=MI_OK:成功
//       得到的序列号放入指定单元
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
	#if 0
    char status ;
    unsigned char i;
    unsigned char ucBits;
    unsigned char ucBytes;
    unsigned char snr_check = 0;
    unsigned char ucCollPosition = 0;
    unsigned char ucTemp;
    unsigned char  ucSNR[5] = {0, 0, 0, 0 ,0};
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    WriteRawRC(RegDecoderControl,0x28);
    ClearBitMask(RegControl,0x08);
    WriteRawRC(RegChannelRedundancy,0x03);
    PcdSetTmo(4);
    

    do
    {
        ucBits = (ucCollPosition) % 8;
        if (ucBits != 0)
        {
             ucBytes = ucCollPosition / 8 + 1;
             WriteRawRC(RegBitFraming, (ucBits << 4) + ucBits);
        }
        else
        {
             ucBytes = ucCollPosition / 8;
        }
	
        MfComData.MfCommand = PCD_TRANSCEIVE;
        MfComData.MfData[0] = PICC_ANTICOLL1;
        MfComData.MfData[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
        for (i=0; i<ucBytes; i++)
	    {
	        MfComData.MfData[i + 2] = ucSNR[i];
	    }
	    MfComData.MfLength = ucBytes + 2;
	
	    status = PcdComTransceive(pi);
	
	    ucTemp = ucSNR[(ucCollPosition / 8)];
	    if (status == MI_COLLERR)
	    {
	        for (i=0; i < 5 - (ucCollPosition / 8); i++)
	        {
		         ucSNR[i + (ucCollPosition / 8)] = MfComData.MfData[i+1];
	        }
	        ucSNR[(ucCollPosition / 8)] |= ucTemp;
	        ucCollPosition = MfComData.MfData[0];
        }
        else if (status == MI_OK)
        {
            for (i=0; i < (MfComData.MfLength / 8); i++)
            {
                 ucSNR[4 - i] = MfComData.MfData[MfComData.MfLength/8 - i - 1];
            }
            ucSNR[(ucCollPosition / 8)] |= ucTemp;
        }
    } while (status == MI_COLLERR);
			
			
    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucSNR[i];
             snr_check ^= ucSNR[i];
         }
         if (snr_check != ucSNR[i])
         {   status = MI_COM_ERR;    }
    }
    
    ClearBitMask(RegDecoderControl,0x20);
    return status;
	#endif
}

/////////////////////////////////////////////////////////////////////
//选定一张卡
//input:g_cSNR=序列号
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr,unsigned char *pSize)
{
	#if 0
    unsigned char i;
    char status;
    unsigned char snr_check = 0;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    WriteRawRC(RegChannelRedundancy,0x0F);
    ClearBitMask(RegControl,0x08);
    PcdSetTmo(4);
    
    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 7;
    MfComData.MfData[0] = PICC_ANTICOLL1;
    MfComData.MfData[1] = 0x70;
    for (i=0; i<4; i++)
    {
    	snr_check ^= *(pSnr+i);
    	MfComData.MfData[i+2] = *(pSnr+i);
    }
    MfComData.MfData[6] = snr_check;

    status = PcdComTransceive(pi);
    
    if (status == MI_OK)
    {
        if (MfComData.MfLength != 0x8)
        {   status = MI_BITCOUNTERR;   }
        else
        {  *pSize = MfComData.MfData[0];  }
    }

    return status;
	#endif
}
