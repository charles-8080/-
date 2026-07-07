/********************************************************************
*********************************************************************
* 文件名称: fft.c													*
* 文件简述：CMSIS-DSP 库的 FFT 调用方法举例 						*
* 创建日期：2024.07.11												*
* 修改日期：2024.07.21												*
* 说    明：CMSIS-DSP 库的 FFT 调用方法，注意同址运算，空间复用		*
*********************************************************************
********************************************************************/

#include "fft.h"
#include "arm_const_structs.h" 		// ARM 定义的 FFT 配置结构体

float32_t FFT_IO_Data[FFT_N << 1];	// 复数 FFT 输入数据、输出数据、取模数据、单边幅度谱数据（采取空间复用，节省空间）
float32_t Mag_max;					// 最大幅度分量
uint32_t Mag_max_index;				// 最大分量频率下标

/****************************************************************************
* 名    称: void DSP_FFT(void)
* 功    能：根据 ADC 缓冲区的数据进行一次 FFT 运算，将计算的单边幅度谱存入 FFT_IO_Data
* 入口参数：无
* 返回参数：无
* 说    明：可灵活调整
****************************************************************************/
void DSP_FFT(void)
{
	// 将需要 FFT 的数据转为 float32_t 型, 并搬移到 FFT_IO_Data 中
	// 复数存储格式：{real[0], imag[0], real[1], imag[1],..}
	for (uint16_t i = 0; i < FFT_N; i++)
	{
		// 实部
#if	(CHANNEL_SIZE == 2)	// 选择一个通道做 FFT
		FFT_IO_Data[i << 1] = pADC1_Data_Buffer[i][0];
#elif (CHANNEL_SIZE == 1)
		FFT_IO_Data[i << 1] = pADC1_Data_Buffer[i];
#endif
		// 虚部
		FFT_IO_Data[(i << 1) + 1] = 0;
	}
		
	// CMSIS-DSP 库文档推荐使用混合基 FFT (arm_cfft_f32)，输出/输出支持同址运算(Processing occurs in-place)
	arm_cfft_f32(&FFT_STRUCT, FFT_IO_Data, 0, 1);
	
	// 幅度谱（双边->单边）
	arm_cmplx_mag_f32(FFT_IO_Data, FFT_IO_Data, FFT_N);	// 双边幅度谱
	for (uint16_t i = 1; i < (FFT_N >> 1); i++)
	{
		FFT_IO_Data[i] = 2 * FFT_IO_Data[i] / FFT_N;	// 单边幅度谱
	}
	FFT_IO_Data[0] = 0;	// 去除直流分量
	
	// 计算幅度最大值
	arm_max_f32(FFT_IO_Data, FFT_N >> 1, &Mag_max, &Mag_max_index);	// 最大幅度分量/频率下标
}
