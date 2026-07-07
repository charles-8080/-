/********************************************************************************
*********************************************************************************
* 文件名称: adc.c																*
* 文件简述：定时器2 触发 ADC1 通道0/1采样，DMA 搬运								*
* 创建日期：2024.07.07															*
* 修改日期：2024.07.21															*
* 说    明：通过 TIM2_Init(marr, mpsc)， 可调节采样率							*
*********************************************************************************
*********************************************************************************/

#include "adc.h"

// 采样数据存储缓冲区
uint16_t ADC1_Data_Buffer[ADC_BUFFER_LEN * CHANNEL_SIZE];

// 单通道情况下, 定义指向缓冲区 ADC1_Data_Buffer 的指针
// 双通道情况下, 定义指向缓冲区 ADC1_Data_Buffer 的二维数组指针, pADC1_Data_Buffer[数据ID][通道ID]
#if CHANNEL_SIZE == 1
	uint16_t (*pADC1_Data_Buffer) = (void*)ADC1_Data_Buffer;
#elif CHANNEL_SIZE == 2
	uint16_t (*pADC1_Data_Buffer)[CHANNEL_SIZE] = (uint16_t(*)[CHANNEL_SIZE]) ((void*)ADC1_Data_Buffer);
#endif

/****************************************************************************
* 名    称: void ADC1_Init(void)
* 功    能：初始化 ADC1
* 入口参数：无
* 返回参数：无
* 说    明：可通过 adc.h 中的宏定义设置通道数
****************************************************************************/
void  ADC1_Init(void)
{	
	/* 使能时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// 使能 GPIOA 时钟 168MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 	// 使能 ADC1 时钟 84MHz

	/* 配置 ADC1 IO口 */
	GPIO_InitTypeDef GPIO_InitStructure;
	
#if (CHANNEL_SIZE == 2)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	// PA0 PA1
#elif (CHANNEL_SIZE == 1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				// PA0
#endif

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;		// 模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	// 不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);				// 初始化
	
	/* 配置 ADC */
	// 复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);	// ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);	// 复位结束
	
	// ADC 常规配置
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;						// 独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;	// 两个采样阶段之间的延迟 5 个时钟
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; 				// DMA
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;						// 预分频 4 分频，ADCCLK = PCLK2/4 = 84/4 = 21Mhz < 36MHz
	ADC_CommonInit(&ADC_CommonInitStructure);										// 初始化
	
	// ADC 配置
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;	// 12位模式
	
	// 扫描模式	
#if (CHANNEL_SIZE == 2)
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;			
#elif (CHANNEL_SIZE == 1)
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
#endif
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;								// 非连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;			// TIM2 触发 ADC1
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;	// 外部上升沿触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;							// 右对齐
	ADC_InitStructure.ADC_NbrOfConversion = CHANNEL_SIZE;							// CHANNEL_SIZE 个转换在规则序列中
	ADC_Init(ADC1, &ADC_InitStructure);												// ADC初始化
	
	// ADC 通道配置
#if (CHANNEL_SIZE == 2)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);
#elif (CHANNEL_SIZE == 1)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);
#endif

	/* 配置 ADC 中的 DMA */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);	
}

/****************************************************************************
* 名    称: void ADC1_DMA2_StartOnce(void)
* 功    能：完成一轮 ADC1 DMA2 采样
* 入口参数：无
* 返回参数：无
****************************************************************************/
void ADC1_DMA2_StartOnce(void)
{
	/* 设置采样率 */
	// 采样率：84 / (marr*mpsc) MHz
	// 单通道最高采样率：21MHz / (最少3+12=15时钟周期) = 1.4MHz
	// 双通道最高采样率：21MHz / (最少3+12+5+3+12=35时钟周期) = 600kHz
	TIM2_Init(2, 70);	// 采样率 = 84MHz/2/70 = 0.6MHz
	
	// 延时稳定
	delay_us(2);
	
	/* 初始化 ADC，启动 ADC */
	ADC1_Init();
	ADC_Cmd(ADC1, ENABLE);
	
	/* 配置 DMA，配置 DMA 中断，启动 DMA */
	USER_DMA_Config(DMA2_Stream0, DMA_Channel_0, DMA_DIR_PeripheralToMemory, (u32)&ADC1->DR, (u32)ADC1_Data_Buffer, ADC_BUFFER_LEN * CHANNEL_SIZE);
	DMA2_Stream0_NVIC_Config();
	USER_DMA_Enable(DMA2_Stream0, ADC_BUFFER_LEN * CHANNEL_SIZE);
		
	/* 开启 TIM2 */
	TIM_Cmd(TIM2, ENABLE);
	
	/* 等待 DMA 搬运完成 */
	while(ADC_DMA_FLAG == 0);
	ADC_DMA_FLAG = 0;
	
	/* 关闭 TIM、ADC、DMA 外设 */
	TIM_Cmd(TIM2, DISABLE);
	ADC_Cmd(ADC1, DISABLE);
	DMA_Cmd(DMA2_Stream0, DISABLE);	
	
	// 延时稳定
	delay_us(2);
}
