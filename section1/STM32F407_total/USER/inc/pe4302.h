#ifndef _PE4302_H_
#define _PE4302_H_

#include "common.h"

#define PE_LE_0					PCout(6)		//PC6接LE接口
#define PE_CLK					PCout(7)		//PC7接CLK接口
#define PE_DAT					PCout(8)		//PC8接DATA接口

#define PE_LE_0_EN      PE_LE_0 = 1
#define PE_LE_0_DIS     PE_LE_0 = 0

#define PE_CLK_0        PE_CLK = 0
#define PE_CLK_1        PE_CLK = 1

#define PE_DAT_0        PE_DAT = 0
#define PE_DAT_1        PE_DAT = 1

void PE_GPIO_Init(void);	//初始化函数
void PE4302_0_Set(u8 db);	//衰减（电压）6dB对应一半	//参数db范围0~63，对应衰减0~31.5dB

#endif
