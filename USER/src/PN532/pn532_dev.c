#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "pn532_dev.h"
#include "nfc.h"

extern u8 PN532_RxBuffer[PN532_BUFFER_LEN];
extern uint16_t PN532_RXCounter;
extern uint16_t PN532_rxnum;
extern __IO u8 flag_rev_finish;

void USART2_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void PN532_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	 
	/* Configure USART2 Tx (PA.2) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART Rx (PA.3) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(USART2, ENABLE);

	USART2_NVIC_Configuration();
}
/*
 * 函数名：TIM2_NVIC_Configuration
 * 描述  ：TIM2中断优先级配置
 * 输入  ：无
 * 输出  ：无	
 */
void TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*TIM_Period--1000   TIM_Prescaler--71 -->中断周期为1ms*/
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period= 720;//720;		 				/* 自动重装载寄存器周期的值(计数值) */
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
    TIM_TimeBaseStructure.TIM_Prescaler= (10000 - 1);			/* 时钟预分频数 72M/72 */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		/* 采样分频 */
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;   /* 向上计数模式 */
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);						/* 清除溢出中断标志 */
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
   // TIM_Cmd(TIM2, ENABLE);																		/* 开启时钟 */
   // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);		/*先关闭等待使用*/ 
	TIM2_NVIC_Configuration();   
}
void PN532_DevConfig(void)
{
	PN532_USART_Config(); 
  	TIM2_Configuration();
}


//定时器中断处理
void TIM2_IRQHandler(void)
{
   TIM_Cmd(TIM2, DISABLE);
   TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
   PN532_rxnum = PN532_RXCounter;
   PN532_RXCounter=0;
   flag_rev_finish=1;
}

//串口2中断处理函数
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {    
        TIM_Cmd(TIM2, ENABLE);
        TIM2->CNT &= 0x0000;    //定时器延时设置：每次接受清零，当接受间隔超过定时器定的值时，进入tim中断，认为一帧接受完成。
        PN532_RxBuffer[PN532_RXCounter++] = USART_ReceiveData(USART2);
		if(PN532_RXCounter == PN532_BUFFER_LEN)
			PN532_RXCounter = 0;
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    /* 发送缓冲区空中断 */
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {   
        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
}
