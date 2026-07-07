/********************************************************************************
*********************************************************************************
* 文件名称: dma.c	 															*
* 文件简述：辅助 adc.c 中的模拟外设实现高效采样									*
* 创建日期：2024.07.07															*
* 修改日期：2024.07.21															*
*********************************************************************************
*********************************************************************************/

#include "dma.h"
#include "audio_analyzer.h"

// 定义 ADC 采样完成标志
__IO uint8_t ADC_DMA_FLAG = 0;

/****************************************************************************
* 名    称: void USER_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, uint32_t chx, uint32_t trdir, uint32_t par, uint32_t mar, uint16_t ndtr)
* 功    能：DMAx的各通道配置
* 入口参数：DMA_Streamx: DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
            chx: DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
			trdir: 传输方向
			par: 外设地址
			mar: 存储器地址
			ndtr: 数据传输量
* 返回参数：无
* 说    明：这边传输形式是固定的, 要根据不同的情况来修改
****************************************************************************/ 
void USER_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, uint32_t chx, uint32_t trdir, uint32_t par, uint32_t mar, uint16_t ndtr)
{ 
	/* 使能DMAy_Streamx时钟 */
	if((uint32_t)DMA_Streamx > (uint32_t)DMA2)	// 得到当前 Stream 是属于 DMA2 还是 DMA1
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);	// DMA2时钟使能 168MHz
	else
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);	// DMA1时钟使能 168MHz

	/* 配置DMA_Stream */
	DMA_DeInit(DMA_Streamx);
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//等待DMA可配置 
	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_Channel = chx;  			// 通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;	// DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;	// DMA 存储器地址
	DMA_InitStructure.DMA_DIR = trdir;				// 传输方向
	DMA_InitStructure.DMA_BufferSize = ndtr;		// 数据传输量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			// 外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						// 存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	// 外设数据长度(半字, 即16bit)
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			// 存储器数据长度(半字, 即16bit)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								// 普通模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						// 最高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;						// 禁用 FIFO
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;					// 存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;			// 外设突发单次传输
	DMA_Init(DMA_Streamx, &DMA_InitStructure);									// 初始化 DMA Stream
}

/****************************************************************************
* 名    称: void DMA2_Stream0_NVIC_Config(void)
* 功    能：DMA2_Stream0 中断配置(启动 DMA_IT_TC 中断)
* 入口参数：无
* 返回参数：无
* 说    明：适用于 ADC1、ADC3
****************************************************************************/
void DMA2_Stream0_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
}

/****************************************************************************
* 名    称: void USER_DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, uint16_t ndtr)
* 功    能：开启一次DMA传输
* 入口参数：DMA_Streamx: DMA数据流, DMA1_Stream0~7 / DMA2_Stream0~7
            ndtr: 数据传输量
* 返回参数：无
* 说    明：无
****************************************************************************/
void USER_DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, uint16_t ndtr)
{
	DMA_Cmd(DMA_Streamx, DISABLE);						// 关闭DMA传输
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	// 确保DMA可以被设置
	DMA_SetCurrDataCounter(DMA_Streamx, ndtr);			// 数据传输量
	DMA_Cmd(DMA_Streamx, ENABLE);						// 开启DMA传输
}

/****************************************************************************
* 名    称: void  DMA2_Stream0_IRQHandler(void)
* 功    能：DMA2_Stream0 中断服务程序
* 返回参数：无
* 说    明：DMA2_Stream0 产生 DMA_IT_TCIF0 中断时, 自动跳转至此函数
****************************************************************************/
void  DMA2_Stream0_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0) != RESET)
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);	// 清除中断标志
		ADC_DMA_FLAG = 1;									// 兼容旧 ADC 采样接口
		AudioAnalyzer_DmaCompleteCallback();					// 音频分析采样完成标志
	}
}
