#ifndef __UART_H
#define __UART_H

#include "common.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////////////	 

#define Str_Len 100	//字符串长度
extern char String[Str_Len];	//格式化字符串

//////////////////////////////////////////////////////////////////////////////////

void uart1_init(u32 bound);
void uart1SendChars(u8 *str, u8 strlen);

void uart4_init(u32 bound);
void uart4SendChars(u8 *str, u8 strlen);

#endif
