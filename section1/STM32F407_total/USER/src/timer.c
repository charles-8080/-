/********************************************************************************
*********************************************************************************
* 文件名称: timer.c																*
* 文件简述：TIM2_Init: 定时器触发 ADC 											*
			
* 创建日期：2024.07.07															*
* 修改日期：2024.07.21															*
* 说    明：无																	*									   						 *
*********************************************************************************
*********************************************************************************/

#include "timer.h"

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////// ADC 采样控制定时器配置 //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/****************************************************************************
* 名    称: TIM2_Init(u16 marr, u16 mpsc)
* 功    能：定时器2 初始化
* 入口参数：marr: 自动重装值
*           mpsc: 时钟预分频数
* 返回参数：无
* 说    明：定时器溢出时间计算方法: Tout(us) = marr * mpsc / 84
			用于控制 ADC1 的采样率 
****************************************************************************/
void TIM2_Init(u16 marr, u16 mpsc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;				 	// 定时器初始化结构体
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			// 使能 TIM2 上的 APB1 时钟（84MHz）
	TIM_TimeBaseInitStruct.TIM_Prescaler = mpsc - 1;				// TIM2 的定时器分频系数
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; 	// TIM2 为向上计数方式
	TIM_TimeBaseInitStruct.TIM_Period = marr - 1;					// TIM2 的自动重装载值
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	 	// 时钟分频因子
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);				// TIM2 初始化
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);		 	// TIM2 更新触发ADC1
	TIM_ARRPreloadConfig(TIM2, ENABLE); 						 	// 允许 TIM2 定时重载
	TIM_Cmd(TIM2, DISABLE);										 	// 失能 TIM2
}

/****************************************************************************
* 名    称: TIM6_Init(u16 marr, u16 mpsc)
* 功    能：定时器6 初始化
* 入口参数：marr: 自动重装值
*           mpsc: 时钟预分频数
* 返回参数：无
* 说    明：定时器溢出时间计算方法: Tout(us) = marr * mpsc / 84
****************************************************************************/
//void TIM6_Init(u16 marr, u16 mpsc)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;				 	// 定时器初始化结构体
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);			// 使能 TIM6 上的 APB1 时钟（84MHz）
//	TIM_TimeBaseInitStruct.TIM_Prescaler = mpsc - 1;				// TIM6 的定时器分频系数
//	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; 	// TIM6 为向上计数方式
//	TIM_TimeBaseInitStruct.TIM_Period = marr - 1;					// TIM6 的自动重装载值
//	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;	 	// 时钟分频因子
//	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);				// TIM6 初始化
//	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);		 	// TIM6 更新触发 DAC1
//	TIM_ARRPreloadConfig(TIM6, ENABLE); 						 	// 允许 TIM6 定时重载
//	TIM_Cmd(TIM6, DISABLE);										 	// 失能 TIM6
//}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// 频率计定时器配置 ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/****************************************************************************
* 名    称: void TIM7_Init(void)
* 功    能：定时器7 初始化，作为频率计的基准时钟
* 入口参数：无
* 返回参数：无
* 说    明：请不要随意更改
****************************************************************************/
void TIM7_Init(void)	
{
	/* TIM7 Init (Base TIM) --> GATE */
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStrcture;
	
	/* Freq of APB1: 168/4 = 42MHz; TIM7 Freq = 2*42 = 84MHz */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);	
	
	/* 	[Gate] Period of TIM7  Reload = (arr + 1)(psc + 1) / 84M  
	 *	[f standard] After prescaler = 84M / (psc + 1)   
	 *	Error smaller -> Gate*fs larger, Gate*fs = arr + 1
	 *	Let Gate Time = 0.1s
	 *	arr = 50000 - 1
	 *	psc = 168 - 1
	 */
	
	TIM_InitStructure.TIM_Period = 50000 - 1;				// 计数周期 -- 50000
	TIM_InitStructure.TIM_Prescaler = 168 - 1;				// 计数频率 -- 84e6 / 168 = 500 kHz
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 不分频
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_InitStructure.TIM_RepetitionCounter = DISABLE;		// 不重复
	TIM_TimeBaseInit(TIM7, &TIM_InitStructure);

	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);	// 清中断
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);	// 开中断

	NVIC_InitStrcture.NVIC_IRQChannel = TIM7_IRQn;				// 中断
	NVIC_InitStrcture.NVIC_IRQChannelPreemptionPriority = 0;	// 抢占优先级 0
	NVIC_InitStrcture.NVIC_IRQChannelSubPriority = 0;			// 响应优先级 0
	NVIC_InitStrcture.NVIC_IRQChannelCmd = ENABLE;				// 开中断
	NVIC_Init(&NVIC_InitStrcture);
	
	TIM_Cmd(TIM7, DISABLE);	// 停止计数
}

/****************************************************************************
* 名    称: void TIM1_ETR_Init(void)
* 功    能：开启 定时器1 的 ETR 功能，实现引脚 PA12 对外部脉冲计数
* 入口参数：无
* 返回参数：无
* 说    明：请不要随意更改
****************************************************************************/
void TIM1_ETR_Init(void)
{
	/* TIM1 ETR Init (Advanced TIM)  ETR Count -- > PA12 */
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStrcture;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);		// TIM1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// GPIOA

	// GPIOA (PA12) 初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;			// PA12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  		// 复用
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  		// 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	// 100 MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_TIM1);

	// TIM1 初始化
	TIM_InitStructure.TIM_Period = 0xFFFF;  				// 计数周期
	TIM_InitStructure.TIM_Prescaler = 0;   					// 分配系数
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 不分频
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseInit(TIM1, &TIM_InitStructure);
	TIM_ETRClockMode2Config(TIM1, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);  // 外部时钟源模式
	
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);	// 清中断
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);	// 开中断
	
	// 中断配置
	NVIC_InitStrcture.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;		// 中断
	NVIC_InitStrcture.NVIC_IRQChannelPreemptionPriority = 0;	// 抢占优先级 0 
	NVIC_InitStrcture.NVIC_IRQChannelSubPriority = 1;		  	// 响应优先级 1
	NVIC_InitStrcture.NVIC_IRQChannelCmd = ENABLE;			  	// 开中断
	NVIC_Init(&NVIC_InitStrcture);

	TIM_Cmd(TIM1, DISABLE);	// 停止计数
}

/****************************************************************************
* 名    称: void TIM7_IRQHandler(void)
* 功    能：定时器7 中断服务程序，用于指示频率计闸门和异常超时
* 入口参数：无
* 返回参数：无
* 说    明：请不要随意更改
****************************************************************************/
void TIM7_IRQHandler(void)
{
	if(TIM7->SR & (1<<0))	  // TIM_GetITStatus(TIM7,TIM_IT_Update) == SET
	{
		timesTIM7_UpOverLoad ++;							// 基准信号计数溢出次数
		if(timesTIM7_UpOverLoad == 11) overtimeFlag = 0;	// 1.1s 外部脉冲还没有来, 判定为超时异常
		TIM7->SR &= ~(1<<0);  								// TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	}
}

/****************************************************************************
* 名    称: void TIM1_UP_TIM10_IRQHandler(void)
* 功    能：定时器1 中断服务程序，用于指示外部脉冲计数溢出
* 入口参数：无
* 返回参数：无
* 说    明：请不要随意更改
****************************************************************************/
void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM1->SR & (1<<0))
	{
		timesTIM1_UpOverLoad++;		// 外部脉冲计数溢出次数
		TIM1->SR &= ~(1<<0);  		// 清中断
	}
}
