#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f10x.h"


//Mini STM32������
//ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���
//����delay_us,delay_ms

void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
void Delay_10us(uint32_t us);
#endif





























