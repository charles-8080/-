/********************************************************************************
*********************************************************************************
* 文件名称: audio_analyzer.c                                                    *
* 文件简述: STM32F407 音频信号分析基础模块                                      *
* 说    明: PA5(ADC1_IN5) + TIM2 TRGO + DMA2_Stream0，1024 点 48 kHz 采样       *
*********************************************************************************
*********************************************************************************/

#include "audio_analyzer.h"
#include "dma.h"
#include "timer.h"
#include "uart.h"
#include <stdio.h>

uint16_t adc_buf[AUDIO_FFT_SIZE];
__IO uint8_t audio_data_ready = 0;
AudioResult_t g_audio_result = {0};

static __IO uint8_t audio_sampling_busy = 0;
static arm_rfft_fast_instance_f32 rfft_instance;
static float32_t fft_input[AUDIO_FFT_SIZE];
static float32_t fft_output[AUDIO_FFT_SIZE];
static float32_t fft_mag[AUDIO_FFT_SIZE / 2];

static void AudioAnalyzer_ADC1_Init(void);
static void AudioAnalyzer_TIM2_48k_Init(void);
static void AudioAnalyzer_DMA_Init(void);
static void AudioAnalyzer_StopSampling(void);
static void AudioAnalyzer_PrintFixed3(const char *label, float value, const char *unit);
static void AudioAnalyzer_PrintFixed1(const char *label, float value, const char *unit);

/****************************************************************************
* 名    称: void AudioAnalyzer_Init(void)
* 功    能: 初始化 FFT 实例和采样外设
****************************************************************************/
void AudioAnalyzer_Init(void)
{
	arm_rfft_fast_init_f32(&rfft_instance, AUDIO_FFT_SIZE);

	AudioAnalyzer_TIM2_48k_Init();
	AudioAnalyzer_ADC1_Init();
	AudioAnalyzer_DMA_Init();
}

/****************************************************************************
* 名    称: void AudioAnalyzer_Start(void)
* 功    能: 启动一轮 1024 点 ADC + DMA 采样
****************************************************************************/
void AudioAnalyzer_Start(void)
{
	audio_data_ready = 0;
	audio_sampling_busy = 1;

	AudioAnalyzer_StopSampling();
	AudioAnalyzer_TIM2_48k_Init();
	AudioAnalyzer_ADC1_Init();
	AudioAnalyzer_DMA_Init();

	DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0 | DMA_FLAG_HTIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_FEIF0);
	USER_DMA_Enable(DMA2_Stream0, AUDIO_FFT_SIZE);

	ADC_Cmd(ADC1, ENABLE);
	TIM_SetCounter(TIM2, 0);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_Cmd(TIM2, ENABLE);
}

/****************************************************************************
* 名    称: void AudioAnalyzer_Process(uint16_t *adc_buf, uint32_t len)
* 功    能: 计算 Vpp、去直流 RMS、1024 点实数 FFT 主频
****************************************************************************/
void AudioAnalyzer_Process(uint16_t *buf, uint32_t len)
{
	uint32_t i;
	float32_t voltage;
	float32_t min_voltage = AUDIO_ADC_REF;
	float32_t max_voltage = 0.0f;
	float32_t sum_voltage = 0.0f;
	float32_t sum_square = 0.0f;
	float32_t max_mag;
	uint32_t max_index;

	if ((buf == 0) || (len != AUDIO_FFT_SIZE)) {
		return;
	}

	for (i = 0; i < len; i++) {
		/* ADC 原始值转换为电压: voltage = adc_value * 3.3 / 4095 */
		voltage = ((float32_t)buf[i]) * AUDIO_ADC_REF / AUDIO_ADC_MAX;
		sum_voltage += voltage;

		if (voltage > max_voltage) {
			max_voltage = voltage;
		}
		if (voltage < min_voltage) {
			min_voltage = voltage;
		}
	}

	/* 平均值即直流偏置，音频输入硬件已偏置到约 1.65 V */
	g_audio_result.dc_offset = sum_voltage / (float32_t)len;
	g_audio_result.vpp = max_voltage - min_voltage;

	for (i = 0; i < len; i++) {
		voltage = ((float32_t)buf[i]) * AUDIO_ADC_REF / AUDIO_ADC_MAX;

		/* 去除直流分量，得到交流采样值 x[i] */
		fft_input[i] = voltage - g_audio_result.dc_offset;
		sum_square += fft_input[i] * fft_input[i];
	}

	/* 交流有效值 RMS = sqrt(sum(x[i] * x[i]) / N) */
	arm_sqrt_f32(sum_square / (float32_t)len, &g_audio_result.rms);

	/* 1024 点实数 FFT，输出为 CMSIS-DSP RFFT fast 打包格式 */
	arm_rfft_fast_f32(&rfft_instance, fft_input, fft_output, 0);

	fft_mag[0] = 0.0f;	/* 忽略直流分量 bin0 */
	arm_cmplx_mag_f32(&fft_output[2], &fft_mag[1], (AUDIO_FFT_SIZE / 2) - 1);

	/* 从 bin1 开始寻找最大幅值谱线，频率分辨率为 48000 / 1024 = 46.875 Hz */
	arm_max_f32(&fft_mag[1], (AUDIO_FFT_SIZE / 2) - 1, &max_mag, &max_index);
	max_index += 1;
	if (max_mag > AUDIO_MIN_FFT_MAG) {
		g_audio_result.freq = ((float32_t)max_index) * AUDIO_SAMPLE_RATE / (float32_t)AUDIO_FFT_SIZE;
	} else {
		g_audio_result.freq = 0.0f;
	}
}

/****************************************************************************
* 名    称: void AudioAnalyzer_PrintResult(void)
* 功    能: 串口打印分析结果
****************************************************************************/
void AudioAnalyzer_PrintResult(void)
{
	AudioAnalyzer_PrintFixed1("Freq: ", g_audio_result.freq, " Hz\r\n");
	AudioAnalyzer_PrintFixed3("Vpp : ", g_audio_result.vpp, " V\r\n");
	AudioAnalyzer_PrintFixed3("RMS : ", g_audio_result.rms, " V\r\n");
	printf("\r\n");
}

/****************************************************************************
* 名    称: void AudioAnalyzer_DmaCompleteCallback(void)
* 功    能: DMA 完成后停止本轮采样并置位，主循环再做 FFT
****************************************************************************/
void AudioAnalyzer_DmaCompleteCallback(void)
{
	if (audio_sampling_busy != 0) {
		AudioAnalyzer_StopSampling();
		audio_sampling_busy = 0;
		audio_data_ready = 1;
	}
}

static void AudioAnalyzer_ADC1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* PA5 -> ADC1_IN5，音频模拟输入 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;	/* ADCCLK = 84 MHz / 4 = 21 MHz */
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_15Cycles);
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
}

static void AudioAnalyzer_TIM2_48k_Init(void)
{
	/* TIM2 时钟 84 MHz，84 MHz / 1750 = 48 kHz */
	TIM2_Init(1750, 1);
}

static void AudioAnalyzer_DMA_Init(void)
{
	USER_DMA_Config(DMA2_Stream0,
	                DMA_Channel_0,
	                DMA_DIR_PeripheralToMemory,
	                (u32)&ADC1->DR,
	                (u32)adc_buf,
	                AUDIO_FFT_SIZE);
	DMA2_Stream0_NVIC_Config();
}

static void AudioAnalyzer_StopSampling(void)
{
	TIM_Cmd(TIM2, DISABLE);
	ADC_Cmd(ADC1, DISABLE);
	DMA_Cmd(DMA2_Stream0, DISABLE);
}

static void AudioAnalyzer_PrintFixed3(const char *label, float value, const char *unit)
{
	int32_t scaled = (int32_t)(value * 1000.0f + ((value >= 0.0f) ? 0.5f : -0.5f));
	int32_t int_part = scaled / 1000;
	int32_t frac_part = scaled % 1000;

	if (frac_part < 0) {
		frac_part = -frac_part;
	}

	printf("%s%ld.%03ld%s", label, (long)int_part, (long)frac_part, unit);
}

static void AudioAnalyzer_PrintFixed1(const char *label, float value, const char *unit)
{
	int32_t scaled = (int32_t)(value * 10.0f + ((value >= 0.0f) ? 0.5f : -0.5f));
	int32_t int_part = scaled / 10;
	int32_t frac_part = scaled % 10;

	if (frac_part < 0) {
		frac_part = -frac_part;
	}

	printf("%s%ld.%01ld%s", label, (long)int_part, (long)frac_part, unit);
}
