#ifndef __ADC_H
#define __ADC_H	

#include "common.h"
#include "dma.h"
#include "timer.h"
#include "fft.h"

//////////////////////////////////////////////////////////////////////////////////	 

#define SAMPLE_FREQ 600.0f			// 采样率(这里的采样率不作用到定时器硬件！请修改 TIM2 溢出率来更改 ADC 采样率)
#define CHANNEL_SIZE 2				// 通道数(单通道: PA0; 双通道: PA0 PA1)
#define ADC_BUFFER_LEN 4096			// ADC 缓冲区长度

// 单通道情况下, 声明指向缓冲区 ADC1_Data_Buffer 的指针
// 双通道情况下, 声明指向缓冲区 ADC1_Data_Buffer 的二维数组指针, pADC1_Data_Buffer[数据ID][通道ID]
#if CHANNEL_SIZE == 1
	extern uint16_t (*pADC1_Data_Buffer);
#elif CHANNEL_SIZE == 2
	#define FFT_SELECT_CHANNEL	0	// 双通道情况下，定义进行 FFT 的通道
	extern uint16_t (*pADC1_Data_Buffer)[CHANNEL_SIZE];
#endif

// 开启一轮 ADC1 DMA2 采样
void ADC1_DMA2_StartOnce(void);	

#endif
