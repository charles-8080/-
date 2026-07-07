#ifndef __LCD_H
#define __LCD_H

#include "common.h"

//////////////////////////////////////////////////////////////////////////////////	 

// 背光引脚
#define	LCD_BACK PCout(3)	// LCD背光 PC3

// LCD 驱动参数
extern  uint16_t  lcd_id;			// LCD ID
extern  uint8_t   dir_flag;			// 横屏还是竖屏控制：0，竖屏；1，横屏。	
extern  uint16_t  lcd_width;		// LCD 宽度
extern  uint16_t  lcd_height;		// LCD 高度
extern  uint16_t  write_gramcmd;	// 写gram指令
extern	uint16_t  setxcmd;			// 设置x坐标指令
extern	uint16_t  setycmd;			// 设置y坐标指令

// LCD 的画笔颜色和背景色	   
extern uint16_t  BRUSH_COLOR;	// 默认红色    
extern uint16_t  BACK_COLOR; 	// 背景颜色.默认为白色

// A12 作为数据命令区分线，设置时 STM32 内部会右移一位对齐
#define  CMD_BASE     ((uint32_t)(0x60000000 | 0x0001FFFE))
#define  DATA_BASE    ((uint32_t)(0x60000000 | 0x00020000))
#define LCD_CMD       (*(uint16_t*)CMD_BASE)
#define LCD_DATA      (*(uint16_t*)DATA_BASE)
	 
// 旋转方向定义
#define	DEGREE_0  		0 
#define	DEGREE_90  		1
#define	DEGREE_180		2
#define	DEGREE_270		3

// 颜色值定义
#define  WHITE		0xFFFF
#define  BLACK		0x0000	
#define  RED		0xF800
#define  GREEN		0x07E0
#define  BLUE		0x001F 
#define  BRED		0XF81F
#define  GRED		0XFFE0
#define  GBLUE		0X07FF
#define  BROWN		0XBC40  
#define  BRRED		0XFC07  
#define  GRAY		0X8430  
#define  MAGENTA	0xF81F
#define  CYAN		0x7FFF
#define  YELLOW		0xFFE0

/* 函数集 */

// 驱动函数
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_Value);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteGRAM(void);

void LCD_Open_Window(uint16_t X0,uint16_t Y0,uint16_t width,uint16_t height);	// 设置显示区域
void Set_Direction(uint8_t direction);					 					 	// 设置显示方向

// 画图函数
void LCD_Init(void);			// 初始化
void LCD_DisplayOn(void);		// 开显示
void LCD_DisplayOff(void);		// 关显示
void LCD_Clear(uint16_t Color);	// 清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);	// 设置光标
void LCD_DrawPoint(uint16_t x,uint16_t y);			// 画点
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		// 画线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);	// 画矩形
void LCD_Color_DrawPoint(uint16_t x,uint16_t y,uint16_t color);				// 颜色画点
void LCD_Color_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);		// 颜色画线
void LCD_Color_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);	// 颜色画矩形
//uint16_t  LCD_GetPoint(uint16_t x,uint16_t y);	//读点

void LCD_Fill_onecolor(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);		// 填充单个颜色
void LCD_Draw_Picture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);		// 填充指定颜色
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t word,uint8_t size);						// 显示一个字符
void LCD_DisplayString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p);						// 显示一个12/16/24字体字符串
void LCD_DisplayString_color(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint16_t brushcolor,uint16_t backcolor);		// 显示一个12/16/24字体自定义颜色的字符串
void LCD_DisplayNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);							// 显示数字
void LCD_DisplayNum_color(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode,uint16_t brushcolor,uint16_t backcolor);	// 显示自定义颜色数字	  	   																			 
							  		 
#endif  
