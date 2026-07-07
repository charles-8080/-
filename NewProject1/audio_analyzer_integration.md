# STM32F407 音频分析仪最小接入说明

当前目录没有 `Core/Src/main.c`、`stm32f4xx_hal_msp.c` 等 CubeMX/HAL 源文件，因此本次只新增可直接移植的 `audio_analyzer.c/.h`。把这两个文件加入你的 HAL 工程后，按下面方式接入。

## 1. main.c 接入点

在 `/* USER CODE BEGIN Includes */` 中加入：

```c
#include "audio_analyzer.h"
```

在 `/* USER CODE BEGIN 2 */` 中，放在 `MX_ADC1_Init()`、`MX_TIM2_Init()`、`MX_USARTx_UART_Init()`、`MX_DMA_Init()` 之后：

```c
AudioAnalyzer_Init();
AudioAnalyzer_Start();
```

在 `while (1)` 的 `/* USER CODE BEGIN WHILE */` 或 `/* USER CODE BEGIN 3 */` 中加入：

```c
if (audio_data_ready) {
    audio_data_ready = 0;
    AudioAnalyzer_Process(adc_buf, AUDIO_FFT_SIZE);
    AudioAnalyzer_PrintResult();
    AudioAnalyzer_Start();   /* 若 DMA 配置为 Normal 模式，处理后重新采一帧 */
}
```

如果你的 DMA 使用 Circular 模式，要避免处理时 DMA 正在覆盖同一个 `adc_buf`。第一阶段建议用 Normal 模式，每 1024 点采满后再重启。

## 2. 外设建议配置

ADC1：

- Channel: `ADC_CHANNEL_5`，即 PA5 / ADC1_IN5
- Resolution: 12 bit
- External trigger: TIM2 TRGO rising edge
- DMA continuous requests: Enable
- Continuous conversion: Disable
- Scan conversion: Disable

TIM2：

- TRGO: Update Event
- 目标采样率: 48 kHz
- 若 TIM2 时钟为 84 MHz，可设置 `PSC = 0`，`ARR = 1749`，因为 `84 MHz / (0 + 1) / (1749 + 1) = 48 kHz`

DMA：

- ADC1 常用 `DMA2_Stream0` / `DMA_CHANNEL_0`
- Direction: Peripheral to Memory
- Peripheral increment: Disable
- Memory increment: Enable
- Peripheral data alignment: Half Word
- Memory data alignment: Half Word
- Mode: Normal
- Priority: High

UART：

- 默认代码使用 `huart1`。如果工程用 `huart2`，把 `audio_analyzer.h` 里的 `AUDIO_UART_HANDLE` 改为 `huart2`，或在编译宏中定义 `AUDIO_UART_HANDLE=huart2`。

## 3. HAL 回调

如果工程里没有自定义 `HAL_ADC_ConvCpltCallback()`，当前 `audio_analyzer.c` 已经提供了回调。

如果工程已有此回调，请把 `audio_analyzer.h` 中：

```c
#define AUDIO_ANALYZER_PROVIDE_HAL_CALLBACK 1
```

改为：

```c
#define AUDIO_ANALYZER_PROVIDE_HAL_CALLBACK 0
```

然后在你已有的回调里加入：

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    AudioAnalyzer_AdcConvCpltCallback(hadc);
}
```

## 4. CMSIS-DSP 配置

需要包含 CMSIS-DSP 头文件和库：

- Include path 增加 CMSIS Core 路径，例如 `Drivers/CMSIS/Include`
- Include path 增加 CMSIS-DSP Include 路径，例如 `Drivers/CMSIS/DSP/Include`
- 工程宏增加 `ARM_MATH_CM4`
- STM32F407 使用 FPU 时建议开启硬件浮点：`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- GCC 工程链接 CMSIS-DSP 库时，选择匹配硬浮点的库，例如 `libarm_cortexM4lf_math.a`
- Keil 可在 Manage Run-Time Environment 中启用 CMSIS DSP，或手动加入对应 DSP 库

如果使用 newlib-nano，当前打印代码没有使用 `%f`，不强制要求 `-u _printf_float`。
