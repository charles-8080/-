#ifndef __FFT_H
#define __FFT_H

#include "common.h"
#include "arm_math.h"
#include "adc.h"

//////////////////////////////////////////////////////////////////////////////////	 

// 复数 FFT 点数定义，arm_cfft_sR_f32_len[FFT_N], FFT_N = [16, 32, 64, ..., 4096]
#define FFT_N 	4096							// FFT 点数
#define FFT_STRUCT 	arm_cfft_sR_f32_len4096		// FFT 配置结构体，见 arm_const_structs.h, 

// 外部变量声明
extern float32_t FFT_IO_Data[FFT_N << 1];	// 复数 FFT 输入数据、输出数据、取模数据、单边幅度谱数据
extern float32_t Mag_max;					// 最大幅度分量
extern uint32_t Mag_max_index;				// 最大分量频率下标

// 计算 ADC 原始数据的 FFT 结果，并求取幅度谱
void DSP_FFT(void);

#endif
