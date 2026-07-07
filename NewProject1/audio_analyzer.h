#ifndef AUDIO_ANALYZER_H
#define AUDIO_ANALYZER_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 音频分析基础参数 */
#define AUDIO_SAMPLE_RATE      48000.0f
#define AUDIO_FFT_SIZE         1024U
#define AUDIO_ADC_REF          3.3f
#define AUDIO_ADC_MAX          4095.0f

/* 默认复用 CubeMX/HAL 生成的外设句柄；如工程使用 huart2，可在这里改为 huart2。 */
#ifndef AUDIO_ADC_HANDLE
#define AUDIO_ADC_HANDLE       hadc1
#endif

#ifndef AUDIO_TIM_HANDLE
#define AUDIO_TIM_HANDLE       htim2
#endif

#ifndef AUDIO_UART_HANDLE
#define AUDIO_UART_HANDLE      huart1
#endif

/* 若工程已有 HAL_ADC_ConvCpltCallback，请将此宏改为 0，并在原回调里调用
 * AudioAnalyzer_AdcConvCpltCallback(hadc)。
 */
#ifndef AUDIO_ANALYZER_PROVIDE_HAL_CALLBACK
#define AUDIO_ANALYZER_PROVIDE_HAL_CALLBACK 1
#endif

typedef struct {
    float vpp;
    float rms;
    float freq;
    float dc_offset;
} AudioResult_t;

extern uint16_t adc_buf[AUDIO_FFT_SIZE];
extern volatile uint8_t audio_data_ready;
extern AudioResult_t audio_result;

void AudioAnalyzer_Init(void);
void AudioAnalyzer_Start(void);
void AudioAnalyzer_Process(uint16_t *adc_buf, uint32_t len);
void AudioAnalyzer_PrintResult(void);
void AudioAnalyzer_AdcConvCpltCallback(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_ANALYZER_H */
