/****************************************************************
*****************************************************************
* 文件名称: led.c												*
* 文件简述：LED 初始化											*
* 创建日期：2024.07.11											*
* 说    明：LED 对应 IO 口初始化									*
*****************************************************************
****************************************************************/

#include "led.h" 

// LED对应IO初始化
void LED_Init(void)
{
	// 使能GPIOB时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

	// PB7/8/9初始化设置
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;	// LED3、LED2和LED1对应IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                  	// 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 	// 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;             	// 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                   	// 上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);                         	// 初始化GPIO
	GPIO_SetBits(GPIOB, GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);			// 灭灯
}
