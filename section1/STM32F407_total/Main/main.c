/****************************************************************************
*****************************************************************************
* 文件名: main.c                                                           *
* 文件描述: 主程序文件                                                     *
* 创建日期: 2024.07.11                                                     *
* 修改日期: 2026.07.07                                                     *
* 说明: 语音信号分析系统主程序版本                                       *
*****************************************************************************
*****************************************************************************/

#include <stdio.h>

#include "uart.h"
#include "led.h"
#include "audio_analyzer.h"

//////////////////////////////////////////////////////////////////////////////

int main(void)
{
	/* 判断优先级组 */
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);

	/* 初始化 */
	delay_init();
	uart4_init(115200);
	LED_Init();
	AudioAnalyzer_Init();

	printf("Audio Analyzer Start\r\n");
	AudioAnalyzer_Start();

///////////////////////////////程序为初始化开始////////////////////////////////
	while(1)
	{
		if(audio_data_ready != 0) {
			audio_data_ready = 0;
			AudioAnalyzer_Process(adc_buf, AUDIO_FFT_SIZE);
			AudioAnalyzer_PrintResult();
			AudioAnalyzer_Start();
			LED1 = !LED1;
		}
	}
}
