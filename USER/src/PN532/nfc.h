#ifndef __NFC_H__
#define __NFC_H__
#include "stm32f10x.h"
#include "pn532_dev.h"

#define PN532_BUFFER_LEN (PN53x_EXTENDED_FRAME__DATA_MAX_LEN + PN53x_EXTENDED_FRAME__OVERHEAD)


/* Error codes */
/** @ingroup error
 * @hideinitializer
 * Success (no error)
 */
#define NFC_SUCCESS			 0
/** @ingroup error
 * @hideinitializer
 * Input / output error, device may not be usable anymore without re-open it
 */
#define NFC_EIO				-1
/** @ingroup error
 * @hideinitializer
 * Invalid argument(s)
 */
#define NFC_EINVARG			-2
/** @ingroup error
 * @hideinitializer
 *  Operation not supported by device
 */
#define NFC_EDEVNOTSUPP			-3
/** @ingroup error
 * @hideinitializer
 * No such device
 */
#define NFC_ENOTSUCHDEV			-4
/** @ingroup error
 * @hideinitializer
 * Buffer overflow
 */
#define NFC_EOVFLOW			-5
/** @ingroup error
 * @hideinitializer
 * Operation timed out
 */
#define NFC_ETIMEOUT			-6
/** @ingroup error
 * @hideinitializer
 * Operation aborted (by user)
 */
#define NFC_EOPABORTED			-7
/** @ingroup error
 * @hideinitializer
 * Not (yet) implemented
 */
#define NFC_ENOTIMPL			-8
/** @ingroup error
 * @hideinitializer
 * Target released
 */
#define NFC_ETGRELEASED			-10
/** @ingroup error
 * @hideinitializer
 * Error while RF transmission
 */
#define NFC_ERFTRANS			-20
/** @ingroup error
 * @hideinitializer
 * MIFARE Classic: authentication failed
 */
#define NFC_EMFCAUTHFAIL		-30
/** @ingroup error
 * @hideinitializer
 * Software error (allocation, file/pipe creation, etc.)
 */
#define NFC_ESOFT			-80
/** @ingroup error
 * @hideinitializer
 * Device's internal chip error
 */
#define NFC_ECHIP			-90


void  PN532_SendData(uint8_t *data,uint8_t length); 						 //��USART1������ length���ȵ�����data
void  CleanBuffer(uint16_t num);									 //���� ǰ ���ٸ��ֽڵ�����

uint8_t 	PN532_SAMConfiguration(uint8_t mode,uint8_t timeout,uint8_t irq); 			 					//���ò���
uint8_t  nfc_InListPassiveTarget(uint8_t maxTag,uint8_t brty,uint8_t *CardID); 						//Ѱ��
uint8_t  nfc_PsdVerify(uint8_t AuthenticationType,uint8_t BlockNo,uint8_t * KeyData,uint8_t *SerialNum); //����֤��Կ    
uint8_t  nfc_read(uint8_t BlockNO,uint8_t *BlockData);											//����
uint8_t  nfc_write(uint8_t BlockNO,uint8_t *WriteData);										//д��
uint8_t 	nfc_getIdentityID(uint8_t *identityid);											//��ȡ���֤
extern uint8_t UID[4];		   
extern uint8_t flag_nfc_status;
extern uint8_t UID_backup[4];
extern uint8_t KeyBuf[6];
extern uint8_t BlockBuf[16];
int pn53x_build_frame(uint8_t *pbtFrame, uint16_t *pszFrame, const uint8_t *pbtData, const uint16_t szData);
int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen);
void delay_10ms(uint16_t n10ms);
uint8_t  nfc_InSelect( uint8_t Tag);

#endif /* __NFC_H__ */
