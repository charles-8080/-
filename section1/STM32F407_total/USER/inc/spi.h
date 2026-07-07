#ifndef __SPI_H
#define __SPI_H
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////	 

// SPI2 CS GPIO
#define SPI2_CS1	PBout(11)
#define SPI2_CS2	PBout(12)

// W25QXX(SPI3_CS)的片选信号
#define	W25QXX_CS PBout(6)	

// 初始化 SPI
void SPI2_Init(void);		
void SPI3_Init(void);

// SPI 读写
uint16_t SPI2_ReadWriteByte(uint16_t writeData);	// SPI2总线读写两个字节
uint8_t SPI3_ReadWriteByte(uint8_t writeData);		// SPI3总线读写一个字节

#endif
