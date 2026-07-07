#ifndef __DAC_H
#define __DAC_H

#include "common.h"
#include "dma.h"

//////////////////////////////////////////////////////////////////////////////////

// DAC 初始化
void DAC1_Init(void);	

// DAC 设置电压
void DAC1_SetVoltage(float voltage);

// DAC 设置寄存器
void DAC1_SetRegister(uint16_t volreg);

#endif
