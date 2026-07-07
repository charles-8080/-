#ifndef _TIMER_H
#define _TIMER_H

#include "common.h"
#include "freqmeas.h"

//////////////////////////////////////////////////////////////////////////////////

// TIM2 ≥ı ºªØ
void TIM2_Init(u16 marr,u16 mpsc);
//void TIM6_Init(u16 marr,u16 mpsc);


void TIM1_ETR_Init(void);
void TIM7_Init(void);

#endif
