/****************************************************************************
*****************************************************************************
* �ļ�����: main.c                                                           *
* �ļ�������������                                                           *
* �������ڣ�2024.07.11                                                       *
* �޸����ڣ�2026.07.07                                                       *
* ˵    ������Ƶ�źŷ�������С�����а汾                                     *
*****************************************************************************
*****************************************************************************/
//
//111
#include <stdio.h>

#include "uart.h"
#include "led.h"
#include "audio_analyzer.h"

//////////////////////////////////////////////////////////////////////////////

int main(void)
{
	/* �ж����ȼ��� */
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);

	/* ��ʼ�� */
	delay_init();
	uart4_init(115200);
	LED_Init();
	AudioAnalyzer_Init();

	printf("Audio Analyzer Start\r\n");
	AudioAnalyzer_Start();

///////////////////////////////����Ϊ��ʼ������////////////////////////////////
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
