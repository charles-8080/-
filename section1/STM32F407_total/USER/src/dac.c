/****************************************************************************
*****************************************************************************
* 文件名称: dac.c															*
* 文件简述：DAC 通道1 配置，输出对应电压										*
* 创建日期：2024.07.07														*
* 修改日期：2024.07.21														*
* 说    明：DAC 输出直流电压（若需输出波形，直接用 DDS 模块，无需使用 DAC）	*
*****************************************************************************
****************************************************************************/

#include "dac.h"

/****************************************************************************
* 名    称: void DAC1_Init(void)
* 功    能：初始化 DAC 配置, 电压输出引脚 PA4
* 入口参数：无
* 返回参数：无
****************************************************************************/
void DAC1_Init(void)
{  
	/* 使能时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// 使能GPIOA时钟 168MHz
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);		// 使能DAC时钟 42MHz

	/* DAC端口配置 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		// PA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;	// 模拟，连接到DAC时同时表示模拟输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	// 内部下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// 初始化

	/* DAC初始化设置 */
	DAC_InitTypeDef DAC_InitType;
	DAC_InitType.DAC_Trigger = DAC_Trigger_None;						// 定时器6触发改成DAC_Trigger_T6_TRGO
	DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None;			// 不使用波形发生
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;// 屏蔽、幅值设置
	DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Enable;			// DAC1 输出缓存
	DAC_Init(DAC_Channel_1,	&DAC_InitType);								// 初始化 DAC1
	
	/* 使能 */
	DAC_Cmd(DAC_Channel_1, ENABLE);   			// 使能DAC通道1
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);  	// 12位右对齐数据格式设置DAC值(初始为0V)
}

/****************************************************************************
* 名    称: void DAC1_SetRegister(uint16_t volreg)
* 功    能：设置 DAC 通道1 输出电压
* 入口参数：volreg, 寄存器值, 0~4095
* 返回参数：无
****************************************************************************/
void DAC1_SetRegister(uint16_t volreg)
{
	// 12位右对齐数据格式设置DAC值
	DAC_SetChannel1Data(DAC_Align_12b_R, volreg);
}

/****************************************************************************
* 名    称: void DAC1_SetVoltage(float vol)
* 功    能：设置 DAC 通道1 输出电压
* 入口参数：vol, 电压值, 0~3.3V
* 返回参数：无
****************************************************************************/
void DAC1_SetVoltage(float vol)
{
	// 按照电压值计算寄存器值
	uint16_t temp = (uint16_t)(4096.0f * vol / 3.3f);
	
	// 12位右对齐数据格式设置 DAC 值
	DAC_SetChannel1Data(DAC_Align_12b_R, temp);
}
