#include "stm32f10x.h"
#include "bsp_led.h"
/*********

 void GreenLedOn(void)
 {
	 GPIO_ResetBits( LED6_PORT,	LED6_PIN );
	 
 }
  void RedLedOff(void)
 {
	 GPIO_SetBits( LED1_PORT,	LED1_PIN );
	 

 }
  void GreenLedOff(void)
 {
	 GPIO_SetBits( LED6_PORT,	LED6_PIN );
 
 }
 void RedLedToggle(void)
 {
	 LED1_PORT->ODR ^= LED1_PIN;
	 
	
 }
***********/
void bsp_led_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN | LED3_PIN |LED4_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(LED1_PIN_PORT, &GPIO_InitStructure);

	GPIO_SetBits(LED1_PIN_PORT, LED1_PIN | LED2_PIN | LED3_PIN |LED4_PIN);
}