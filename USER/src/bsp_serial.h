#ifndef __BSP_SERIAL_H
#define __BSP_SERIAL_H

void bsp_serial_config(void);
void UART_Send_Byte(unsigned char dat);
void u8tostr(unsigned char dat) ;
void UART_Send_Str(char *cBuf);
void UART_Send_Enter(void);
void UART_Put_Num(unsigned char * dat,unsigned char len);
void UART_Send_Data(unsigned char * dat,unsigned char len);
unsigned char  SendUartStatus(unsigned int sta);
  
#endif
