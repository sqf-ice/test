#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "stm32f10x.h" 
 
#define  LED1_PIN               GPIO_Pin_0
#define  LED1_PIN_PORT          GPIOC
#define  LED2_PIN               GPIO_Pin_1
#define  LED2_PIN_PORT          GPIOC
#define  LED3_PIN               GPIO_Pin_2
#define  LED3_PIN_PORT          GPIOC
#define  LED4_PIN               GPIO_Pin_3
#define  LED4_PIN_PORT          GPIOC


#define LED1_ON   GPIO_ResetBits(LED1_PIN_PORT,LED1_PIN)
#define LED2_ON   GPIO_ResetBits(LED2_PIN_PORT,LED2_PIN)
#define LED3_ON   GPIO_ResetBits(LED3_PIN_PORT,LED3_PIN)
#define LED4_ON   GPIO_ResetBits(LED4_PIN_PORT,LED4_PIN)


#define LED1_OFF   GPIO_SetBits(LED1_PIN_PORT,LED1_PIN)
#define LED2_OFF   GPIO_SetBits(LED2_PIN_PORT,LED2_PIN)
#define LED3_OFF   GPIO_SetBits(LED3_PIN_PORT,LED3_PIN)
#define LED4_OFF   GPIO_SetBits(LED4_PIN_PORT,LED4_PIN)

#define LED1_TOGGLE   (LED1_PIN_PORT->ODR ^= LED1_PIN)
#define LED2_TOGGLE   (LED2_PIN_PORT->ODR ^= LED2_PIN)
#define LED3_TOGGLE   (LED3_PIN_PORT->ODR ^= LED3_PIN)
#define LED4_TOGGLE   (LED4_PIN_PORT->ODR ^= LED4_PIN)
void bsp_led_config(void);

#endif   