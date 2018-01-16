#ifndef __PN532_DEV_H
#define __PN532_DEV_H
// Miscellaneous
#define Diagnose 0x00
#define GetFirmwareVersion 0x02
#define GetGeneralStatus 0x04
#define ReadRegister 0x06
#define WriteRegister 0x08
#define ReadGPIO 0x0C
#define WriteGPIO 0x0E
#define SetSerialBaudRate 0x10
#define SetParameters 0x12
#define SAMConfiguration 0x14
#define PowerDown 0x16
#define AlparCommandForTDA 0x18
// RC-S360 has another command 0x18 for reset &..?

// RF communication
#define RFConfiguration 0x32
#define RFRegulationTest 0x58

// Initiator
#define InJumpForDEP 0x56
#define InJumpForPSL 0x46
#define InListPassiveTarget 0x4A
#define InATR 0x50
#define InPSL 0x4E
#define InDataExchange 0x40
#define InCommunicateThru 0x42
#define InQuartetByteExchange 0x38
#define InDeselect 0x44
#define InRelease 0x52
#define InSelect 0x54
#define InActivateDeactivatePaypass 0x48
#define InAutoPoll 0x60

// Target
#define TgInitAsTarget 0x8C
#define TgSetGeneralBytes 0x92
#define TgGetData 0x86
#define TgSetData 0x8E
#define TgSetDataSecure 0x96
#define TgSetMetaData 0x94
#define TgSetMetaDataSecure 0x98
#define TgGetInitiatorCommand 0x88
#define TgResponseToInitiator 0x90
#define TgGetTargetStatus 0x8A

#  define PN53x_NORMAL_FRAME__DATA_MAX_LEN              254
#  define PN53x_NORMAL_FRAME__OVERHEAD                  8
#  define PN53x_EXTENDED_FRAME__DATA_MAX_LEN            264
#  define PN53x_EXTENDED_FRAME__OVERHEAD                11
#  define PN53x_ACK_FRAME__LEN                          6



// Register addresses
#define PN53X_REG_Control_switch_rng 0x6106
#define PN53X_REG_CIU_Mode 0x6301
#define PN53X_REG_CIU_TxMode 0x6302
#define PN53X_REG_CIU_RxMode 0x6303
#define PN53X_REG_CIU_TxControl 0x6304
#define PN53X_REG_CIU_TxAuto 0x6305
#define PN53X_REG_CIU_TxSel 0x6306
#define PN53X_REG_CIU_RxSel 0x6307
#define PN53X_REG_CIU_RxThreshold 0x6308
#define PN53X_REG_CIU_Demod 0x6309
#define PN53X_REG_CIU_FelNFC1 0x630A
#define PN53X_REG_CIU_FelNFC2 0x630B
#define PN53X_REG_CIU_MifNFC 0x630C
#define PN53X_REG_CIU_ManualRCV 0x630D
#define PN53X_REG_CIU_TypeB 0x630E
// #define PN53X_REG_- 0x630F
// #define PN53X_REG_- 0x6310
#define PN53X_REG_CIU_CRCResultMSB 0x6311
#define PN53X_REG_CIU_CRCResultLSB 0x6312
#define PN53X_REG_CIU_GsNOFF 0x6313
#define PN53X_REG_CIU_ModWidth 0x6314
#define PN53X_REG_CIU_TxBitPhase 0x6315
#define PN53X_REG_CIU_RFCfg 0x6316
#define PN53X_REG_CIU_GsNOn 0x6317
#define PN53X_REG_CIU_CWGsP 0x6318
#define PN53X_REG_CIU_ModGsP 0x6319
#define PN53X_REG_CIU_TMode 0x631A
#define PN53X_REG_CIU_TPrescaler 0x631B
#define PN53X_REG_CIU_TReloadVal_hi 0x631C
#define PN53X_REG_CIU_TReloadVal_lo 0x631D
#define PN53X_REG_CIU_TCounterVal_hi 0x631E
#define PN53X_REG_CIU_TCounterVal_lo 0x631F
// #define PN53X_REG_- 0x6320
#define PN53X_REG_CIU_TestSel1 0x6321
#define PN53X_REG_CIU_TestSel2 0x6322
#define PN53X_REG_CIU_TestPinEn 0x6323
#define PN53X_REG_CIU_TestPinValue 0x6324
#define PN53X_REG_CIU_TestBus 0x6325
#define PN53X_REG_CIU_AutoTest 0x6326
#define PN53X_REG_CIU_Version 0x6327
#define PN53X_REG_CIU_AnalogTest 0x6328
#define PN53X_REG_CIU_TestDAC1 0x6329
#define PN53X_REG_CIU_TestDAC2 0x632A
#define PN53X_REG_CIU_TestADC 0x632B
// #define PN53X_REG_- 0x632C
// #define PN53X_REG_- 0x632D
// #define PN53X_REG_- 0x632E
#define PN53X_REG_CIU_RFlevelDet 0x632F
#define PN53X_REG_CIU_SIC_CLK_en 0x6330
#define PN53X_REG_CIU_Command 0x6331
#define PN53X_REG_CIU_CommIEn 0x6332
#define PN53X_REG_CIU_DivIEn 0x6333
#define PN53X_REG_CIU_CommIrq 0x6334
#define PN53X_REG_CIU_DivIrq 0x6335
#define PN53X_REG_CIU_Error 0x6336
#define PN53X_REG_CIU_Status1 0x6337
#define PN53X_REG_CIU_Status2 0x6338
#define PN53X_REG_CIU_FIFOData 0x6339
#define PN53X_REG_CIU_FIFOLevel 0x633A
#define PN53X_REG_CIU_WaterLevel 0x633B
#define PN53X_REG_CIU_Control 0x633C
#define PN53X_REG_CIU_BitFraming 0x633D
#define PN53X_REG_CIU_Coll 0x633E

#define PN53X_SFR_P3 0xFFB0

#define PN53X_SFR_P3CFGA 0xFFFC
#define PN53X_SFR_P3CFGB 0xFFFD
#define PN53X_SFR_P7CFGA 0xFFF4
#define PN53X_SFR_P7CFGB 0xFFF5
#define PN53X_SFR_P7 0xFFF7

/* PN53x specific errors */
#define ETIMEOUT	0x01
#define ECRC		0x02
#define EPARITY		0x03
#define EBITCOUNT	0x04
#define EFRAMING	0x05
#define EBITCOLL	0x06
#define ESMALLBUF	0x07
#define EBUFOVF		0x09
#define ERFTIMEOUT	0x0a
#define ERFPROTO	0x0b
#define EOVHEAT		0x0d
#define EINBUFOVF	0x0e
#define EINVPARAM	0x10
#define EDEPUNKCMD	0x12
#define EINVRXFRAM	0x13
#define EMFAUTH		0x14
#define ENSECNOTSUPP	0x18	// PN533 only
#define EBCC			0x23
#define EDEPINVSTATE	0x25
#define EOPNOTALL	0x26
#define ECMD		0x27
#define ETGREL		0x29
#define ECID		0x2a
#define ECDISCARDED	0x2b
#define ENFCID3		0x2c
#define EOVCURRENT	0x2d
#define ENAD		0x2e

// Registers and symbols masks used to covers parts within a register
//   PN53X_REG_CIU_TxMode
#  define SYMBOL_TX_CRC_ENABLE      0x80
#  define SYMBOL_TX_SPEED           0x70
// TX_FRAMING bits explanation:
//   00 : ISO/IEC 14443A/MIFARE and Passive Communication mode 106 kbit/s
//   01 : Active Communication mode
//   10 : FeliCa and Passive Communication mode at 212 kbit/s and 424 kbit/s
//   11 : ISO/IEC 14443B
#  define SYMBOL_TX_FRAMING         0x03

//   PN53X_REG_Control_switch_rng
#  define SYMBOL_CURLIMOFF          0x08     /* When set to 1, the 100 mA current limitations is desactivated. */
#  define SYMBOL_SIC_SWITCH_EN      0x10     /* When set to logic 1, the SVDD switch is enabled and the SVDD output delivers power to secure IC and internal pads (SIGIN, SIGOUT and P34). */
#  define SYMBOL_RANDOM_DATAREADY   0x02     /* When set to logic 1, a new random number is available. */

//   PN53X_REG_CIU_RxMode
#  define SYMBOL_RX_CRC_ENABLE      0x80
#  define SYMBOL_RX_SPEED           0x70
#  define SYMBOL_RX_NO_ERROR        0x08
#  define SYMBOL_RX_MULTIPLE        0x04
// RX_FRAMING follow same scheme than TX_FRAMING
#  define SYMBOL_RX_FRAMING         0x03

//   PN53X_REG_CIU_TxAuto
#  define SYMBOL_FORCE_100_ASK      0x40
#  define SYMBOL_AUTO_WAKE_UP       0x20
#  define SYMBOL_INITIAL_RF_ON      0x04

//   PN53X_REG_CIU_ManualRCV
#  define SYMBOL_PARITY_DISABLE     0x10

//   PN53X_REG_CIU_TMode
#  define SYMBOL_TAUTO              0x80
#  define SYMBOL_TPRESCALERHI       0x0F

//   PN53X_REG_CIU_TPrescaler
#  define SYMBOL_TPRESCALERLO       0xFF

//   PN53X_REG_CIU_Command
#  define SYMBOL_COMMAND            0x0F
#  define SYMBOL_COMMAND_TRANSCEIVE 0xC

//   PN53X_REG_CIU_Status2
#  define SYMBOL_MF_CRYPTO1_ON      0x08

//   PN53X_REG_CIU_FIFOLevel
#  define SYMBOL_FLUSH_BUFFER       0x80
#  define SYMBOL_FIFO_LEVEL         0x7F

//   PN53X_REG_CIU_Control
#  define SYMBOL_INITIATOR          0x10
#  define SYMBOL_RX_LAST_BITS       0x07

//   PN53X_REG_CIU_BitFraming
#  define SYMBOL_START_SEND         0x80
#  define SYMBOL_RX_ALIGN           0x70
#  define SYMBOL_TX_LAST_BITS       0x07

// PN53X Support Byte flags
#define SUPPORT_ISO14443A             0x01
#define SUPPORT_ISO14443B             0x02
#define SUPPORT_ISO18092              0x04

// Internal parameters flags
#  define PARAM_NONE                  0x00
#  define PARAM_NAD_USED              0x01
#  define PARAM_DID_USED              0x02
#  define PARAM_AUTO_ATR_RES          0x04
#  define PARAM_AUTO_RATS             0x10
#  define PARAM_14443_4_PICC          0x20 /* Only for PN532 */
#  define PARAM_NFC_SECURE            0x20 /* Only for PN533 */
#  define PARAM_NO_AMBLE              0x40 /* Only for PN532 */

// Radio Field Configure Items           // Configuration Data length
#  define RFCI_FIELD                  0x01      //  1
#  define RFCI_TIMING                 0x02      //  3
#  define RFCI_RETRY_DATA             0x04      //  1
#  define RFCI_RETRY_SELECT           0x05      //  3
#  define RFCI_ANALOG_TYPE_A_106      0x0A      // 11
#  define RFCI_ANALOG_TYPE_A_212_424  0x0B      //  8
#  define RFCI_ANALOG_TYPE_B          0x0C      //  3
#  define RFCI_ANALOG_TYPE_14443_4    0x0D      //  9

/**
 * @enum pn53x_power_mode
 * @brief PN53x power mode enumeration
 */
typedef enum {
  NORMAL,	// In that case, there is no power saved but the PN53x reacts as fast as possible on the host controller interface.
  POWERDOWN,	// Only on PN532, need to be wake up to process commands with a long preamble
  LOWVBAT	// Only on PN532, need to be wake up to process commands with a long preamble and SAMConfiguration command
} pn53x_power_mode;

/**
 * @enum pn53x_operating_mode
 * @brief PN53x operatin mode enumeration
 */
typedef enum {
  IDLE,
  INITIATOR,
  TARGET,
} pn53x_operating_mode;

/**
 * @enum pn532_sam_mode
 * @brief PN532 SAM mode enumeration
 */
typedef enum {
  PSM_NORMAL = 0x01,
  PSM_VIRTUAL_CARD = 0x02,
  PSM_WIRED_CARD = 0x03,
  PSM_DUAL_CARD = 0x04
} pn532_sam_mode;

#define PN53X_CACHE_REGISTER_MIN_ADDRESS 	PN53X_REG_CIU_Mode
#define PN53X_CACHE_REGISTER_MAX_ADDRESS 	PN53X_REG_CIU_Coll
#define PN53X_CACHE_REGISTER_SIZE 		((PN53X_CACHE_REGISTER_MAX_ADDRESS - PN53X_CACHE_REGISTER_MIN_ADDRESS) + 1)



void PN532_DevConfig(void);

#endif
