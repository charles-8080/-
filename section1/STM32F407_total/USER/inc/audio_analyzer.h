#ifndef __AUDIO_ANALYZER_H
#define __AUDIO_ANALYZER_H

#include "common.h"
#include "arm_math.h"

/* 音频分析参数 */
#define AUDIO_SAMPLE_RATE       48000.0f
#define AUDIO_FFT_SIZE          1024u
#define AUDIO_ADC_REF           3.3f
#define AUDIO_ADC_MAX           4095.0f
#define AUDIO_MIN_FFT_MAG       1.0e-6f

typedef struct {
	float vpp;
	float rms;
	float freq;
	float dc_offset;
} AudioResult_t;

extern uint16_t adc_buf[AUDIO_FFT_SIZE];
extern __IO uint8_t audio_data_ready;
extern AudioResult_t g_audio_result;

void AudioAnalyzer_Init(void);
void AudioAnalyzer_Start(void);
void AudioAnalyzer_Process(uint16_t *adc_buf, uint32_t len);
void AudioAnalyzer_PrintResult(void);

/* DMA2_Stream0 采样完成中断中调用：只停采样并置位标志，不做 FFT 运算 */
void AudioAnalyzer_DmaCompleteCallback(void);

#endif
