#ifndef _AD9959_H
#define _AD9959_H

#include <stdbool.h>
#include "common.h"

//引脚接口，其余接地
#define AD9959_RESET  PCout(13)			//RESET  PC13
#define AD9959_SDIO0  PEout(0) 			//SDIO0  PE0
#define AD9959_SDIO1  PEout(1) 			//SDIO1  PE1
#define AD9959_SDIO2  PEout(2) 			//SDIO2  PE2
#define AD9959_SDIO3  PEout(3) 			//SDIO3  PE3
#define AD9959_CS     PEout(4) 			//CS     PE4
#define AD9959_SCLK   PEout(5) 			//SCLK   PE5
#define AD9959_UPDATE PEout(6) 			//UPDATE PE6

//通道定义
typedef enum {
	AD9959_CHANNEL_0 = 0x10,
	AD9959_CHANNEL_1 = 0x20,
	AD9959_CHANNEL_2 = 0x40,
	AD9959_CHANNEL_3 = 0x80,
}AD9959_CHANNEL;
#define IS_AD9959_CHANNEL(CHANNEL) (((CHANNEL)==AD9959_CHANNEL_0)||((CHANNEL)==AD9959_CHANNEL_1)||((CHANNEL)==AD9959_CHANNEL_2)||((CHANNEL)==AD9959_CHANNEL_3))

//寄存器定义
typedef enum {
	AD9959_REG_CSR   = 0x00,	//CSR		通道选择寄存器
	AD9959_REG_FR1   = 0x01,	//FR1		功能寄存器1
	AD9959_REG_FR2   = 0x02,	//FR2		功能寄存器2
	AD9959_REG_CFR   = 0x03,	//CFR		通道功能寄存器
	
	AD9959_REG_CFTW0 = 0x04,	//CTW0	通道频率转换字寄存器
	AD9959_REG_CPOW0 = 0x05,	//CPW0	通道相位转换字寄存器
	AD9959_REG_ACR   = 0x06,	//ACR		幅度控制寄存器
	
	AD9959_REG_LSRR  = 0x07,	//LSR		通道线性扫描寄存器
	AD9959_REG_RDW   = 0x08,	//RDW		通道线性向上扫描寄存器
	AD9959_REG_FDW   = 0x09,	//FDW		通道线性向下扫描寄存器
}AD9959_REG_ADDR;
#define IS_AD9959_REG_ADDR(REG_ADDR) (((REG_ADDR)==AD9959_REG_CSR)||((REG_ADDR)==AD9959_REG_FR1)||((REG_ADDR)==AD9959_REG_FR2)||((REG_ADDR)==AD9959_REG_CFR)||((REG_ADDR)==AD9959_REG_CFTW0)||((REG_ADDR)==AD9959_REG_CPOW0)||((REG_ADDR)==AD9959_REG_ACR)||((REG_ADDR)==AD9959_REG_LSRR)||((REG_ADDR)==AD9959_REG_RDW)||((REG_ADDR)==AD9959_REG_FDW))

//函数声明
void AD9959_Init(void);
void AD9959_write_frequency(AD9959_CHANNEL channel, uint32_t Freq);
void AD9959_write_phase(AD9959_CHANNEL channel, uint16_t Pha);
void AD9959_write_amplitude(AD9959_CHANNEL channel, uint16_t amplitude);

#endif
