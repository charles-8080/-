/****************************************************************************
*****************************************************************************
* 文件名称: main.c                                                           *
* 文件简述：主函数                                                           *
* 创建日期：2024.07.11                                                       *
* 修改日期：2026.07.07                                                       *
* 说    明：音频信号分析仪最小可运行版本                                     *
*****************************************************************************
*****************************************************************************/
//提交到github
#include <stdio.h>

#include "uart.h"
#include "led.h"
#include "audio_analyzer.h"

//////////////////////////////////////////////////////////////////////////////

int main(void)
{
	/* 中断优先级组 */
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);

	/* 初始化 */
	delay_init();
	uart4_init(115200);
	LED_Init();
	AudioAnalyzer_Init();

	printf("Audio Analyzer Start\r\n");
	AudioAnalyzer_Start();

///////////////////////////////以上为初始化函数////////////////////////////////
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
