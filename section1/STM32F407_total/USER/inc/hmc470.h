#ifndef _HMC470_H
#define _HMC470_H

#include "common.h"

//最左边GND接单片机GND
//PC6接1
//PC7接2
//PC8接4
//PD12接8
//PD13接16
////////////////////////////////////////////////////////////////

void HMC470_Init(void);	//初始化函数
void C_gain (int gain);	//衰减（电压）6dB对应一半

#endif
