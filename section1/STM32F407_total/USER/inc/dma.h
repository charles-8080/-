#ifndef __DMA_H
#define	__DMA_H

#include "common.h"
#include "adc.h"

//////////////////////////////////////////////////////////////////////////////////

// 声明 ADC 采样完成标志为外部变量
extern __IO uint8_t ADC_DMA_FLAG;	

// 配置 DMAx_CHx
void USER_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, uint32_t chx, uint32_t trdir, uint32_t par, uint32_t mar, uint16_t ndtr); 

// DMA2_Stream0 中断配置(启动 DMA_IT_TC 中断), 适用于 ADC1 ADC3 
void DMA2_Stream0_NVIC_Config(void);

// 使能 DMA 传输
void USER_DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, uint16_t ndtr);	

#endif
