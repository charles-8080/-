#include "audio_analyzer.h"
//推送
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "arm_math.h"

#ifndef __weak
#define __weak __attribute__((weak))
#endif

extern ADC_HandleTypeDef AUDIO_ADC_HANDLE;
extern TIM_HandleTypeDef AUDIO_TIM_HANDLE;
extern UART_HandleTypeDef AUDIO_UART_HANDLE;

uint16_t adc_buf[AUDIO_FFT_SIZE];
volatile uint8_t audio_data_ready = 0U;
AudioResult_t audio_result = {0.0f, 0.0f, 0.0f, 0.0f};

static arm_rfft_fast_instance_f32 rfft;
static float32_t fft_input[AUDIO_FFT_SIZE];
static float32_t fft_output[AUDIO_FFT_SIZE];
static float32_t fft_mag[AUDIO_FFT_SIZE / 2U];
static uint8_t rfft_ready = 0U;

static void AudioAnalyzer_UartPrint(const char *text)
{
    if (text == NULL) {
        return;
    }

    (void)HAL_UART_Transmit(&AUDIO_UART_HANDLE,
                            (uint8_t *)text,
                            (uint16_t)strlen(text),
                            100U);
}

void AudioAnalyzer_Init(void)
{
    arm_status status;

    audio_data_ready = 0U;
    audio_result.vpp = 0.0f;
    audio_result.rms = 0.0f;
    audio_result.freq = 0.0f;
    audio_result.dc_offset = 0.0f;

    status = arm_rfft_fast_init_f32(&rfft, AUDIO_FFT_SIZE);
    rfft_ready = (status == ARM_MATH_SUCCESS) ? 1U : 0U;

    if (rfft_ready == 0U) {
        AudioAnalyzer_UartPrint("Audio FFT init failed\r\n");
    }
}

void AudioAnalyzer_Start(void)
{
    audio_data_ready = 0U;

    /* TIM2 作为 ADC 外部触发源。先启动 ADC DMA，再启动定时器触发采样。 */
    (void)HAL_ADC_Start_DMA(&AUDIO_ADC_HANDLE, (uint32_t *)adc_buf, AUDIO_FFT_SIZE);
    (void)HAL_TIM_Base_Start(&AUDIO_TIM_HANDLE);
}

void AudioAnalyzer_Process(uint16_t *buffer, uint32_t len)
{
    uint32_t i;
    float32_t sum_voltage = 0.0f;
    float32_t sum_square = 0.0f;
    float32_t voltage;
    float32_t min_voltage = AUDIO_ADC_REF;
    float32_t max_voltage = 0.0f;
    float32_t max_mag = 0.0f;
    uint32_t max_index = 0U;

    if ((buffer == NULL) || (len != AUDIO_FFT_SIZE) || (rfft_ready == 0U)) {
        return;
    }

    /* 第一次遍历：ADC 原始值转换为电压，同时统计直流平均值和峰峰值。 */
    for (i = 0U; i < AUDIO_FFT_SIZE; i++) {
        voltage = ((float32_t)buffer[i] * AUDIO_ADC_REF) / AUDIO_ADC_MAX;

        sum_voltage += voltage;
        if (voltage < min_voltage) {
            min_voltage = voltage;
        }
        if (voltage > max_voltage) {
            max_voltage = voltage;
        }

        fft_input[i] = voltage;
    }

    audio_result.dc_offset = sum_voltage / (float32_t)AUDIO_FFT_SIZE;
    audio_result.vpp = max_voltage - min_voltage;

    /* 第二次遍历：去除 1.65 V 左右直流偏置，得到交流分量，并计算交流 RMS。 */
    for (i = 0U; i < AUDIO_FFT_SIZE; i++) {
        fft_input[i] -= audio_result.dc_offset;
        sum_square += fft_input[i] * fft_input[i];
    }

    audio_result.rms = sqrtf(sum_square / (float32_t)AUDIO_FFT_SIZE);

    /* 1024 点实数 FFT。输出中 bin0 为直流分量，后续复数频点用于主频搜索。 */
    arm_rfft_fast_f32(&rfft, fft_input, fft_output, 0U);

    fft_mag[0] = fabsf(fft_output[0]);
    for (i = 1U; i < (AUDIO_FFT_SIZE / 2U); i++) {
        float32_t real = fft_output[2U * i];
        float32_t imag = fft_output[(2U * i) + 1U];
        fft_mag[i] = sqrtf((real * real) + (imag * imag));
    }

    /* 忽略直流 bin0，从 bin1 开始寻找最大幅值谱线。 */
    for (i = 1U; i < (AUDIO_FFT_SIZE / 2U); i++) {
        if (fft_mag[i] > max_mag) {
            max_mag = fft_mag[i];
            max_index = i;
        }
    }

    /* 主频 = 最大谱线序号 * 采样率 / FFT 点数，初版分辨率为 46.875 Hz。 */
    audio_result.freq = ((float32_t)max_index * AUDIO_SAMPLE_RATE) / (float32_t)AUDIO_FFT_SIZE;
}

void AudioAnalyzer_PrintResult(void)
{
    char line[96];
    uint32_t freq_hz;
    uint32_t vpp_mv;
    uint32_t rms_mv;

    freq_hz = (uint32_t)(audio_result.freq + 0.5f);
    vpp_mv = (uint32_t)((audio_result.vpp * 1000.0f) + 0.5f);
    rms_mv = (uint32_t)((audio_result.rms * 1000.0f) + 0.5f);

    /* 避免依赖 printf 浮点支持，用 mV 整数拼出 x.xxx V。 */
    (void)snprintf(line, sizeof(line),
                   "Freq: %lu Hz\r\n"
                   "Vpp : %lu.%03lu V\r\n"
                   "RMS : %lu.%03lu V\r\n\r\n",
                   (unsigned long)freq_hz,
                   (unsigned long)(vpp_mv / 1000U),
                   (unsigned long)(vpp_mv % 1000U),
                   (unsigned long)(rms_mv / 1000U),
                   (unsigned long)(rms_mv % 1000U));

    AudioAnalyzer_UartPrint(line);
}

void AudioAnalyzer_AdcConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &AUDIO_ADC_HANDLE) {
        audio_data_ready = 1U;
    }
}

#if AUDIO_ANALYZER_PROVIDE_HAL_CALLBACK
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    AudioAnalyzer_AdcConvCpltCallback(hadc);
}
#endif
