#include "hmc470.h"

//-----------------------------------------------------------------
// void HMC470_Init(void)
//-----------------------------------------------------------------
// HMC470模块初始化
// 函数功能:对IO口进行配置
// 入口参数: 无
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
//-----------------------------------------------------------------
void HMC470_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                  	// 普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 	// 推挽输出	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;								// 时钟50MHz
	GPIO_Init(GPIOC,&GPIO_InitStructure);
  GPIO_SetBits(GPIOC,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);				
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
  GPIO_SetBits(GPIOD,GPIO_Pin_12 | GPIO_Pin_13);
}

//-----------------------------------------------------------------
//   void Basic_Init(int in)
//-----------------------------------------------------------------
// 函数功能: 实现0dB到3dB衰减倍数的输出设置
// 入口参数: 有符号整型
// 返回参数: 有符号整型
// 全局变量: 无
// 调用模块: 无
// 注意事项: 无
//-----------------------------------------------------------------
void Basic_Init(int in)
{
	switch(in) {
		case 0: break;
		case 1: GPIO_ResetBits(GPIOC,GPIO_Pin_6);break;
		case 2: GPIO_ResetBits(GPIOC,GPIO_Pin_7);break;
		case 3: GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7);break;
		default : ;
	}
}

//-----------------------------------------------------------------
//   void C_gain (int gain)
//-----------------------------------------------------------------
// 函数功能: 实现0dB到31dB增益控制
// 入口参数: 有符号整型
// 返回参数: 有符号整型
// 全局变量: 无
// 调用模块:Basic_Init(int in)
// 注意事项: 无
//-----------------------------------------------------------------
void C_gain (int gain)
{	
	if(gain>=0&&gain<=3) {
		switch(gain) {
			case 0: GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);
							GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
							break;
			case 1: GPIO_SetBits(GPIOC,GPIO_Pin_7|GPIO_Pin_8);
							GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
							GPIO_ResetBits(GPIOC,GPIO_Pin_6);
							break;
			case 2: GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_8);
							GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
							GPIO_ResetBits(GPIOC,GPIO_Pin_7);
							break;
			case 3: GPIO_SetBits(GPIOC,GPIO_Pin_8);
							GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
							GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7);
							break;
			default: ;
		}
  }
	else if(gain>=4&&gain<=7) {
		GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7);
		GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
		GPIO_ResetBits(GPIOC,GPIO_Pin_8);
		gain-=4;
		Basic_Init(gain);
  }
	else if(gain>=8&&gain<=15) {
		GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);
		GPIO_SetBits(GPIOD,GPIO_Pin_13);
		GPIO_ResetBits(GPIOD,GPIO_Pin_12);
		gain-=8;
		if(gain>=0&&gain<=3)
			Basic_Init(gain);
		else if(gain>=4&&gain<=7) {
			GPIO_ResetBits(GPIOC,GPIO_Pin_8);
			gain-=4;
			Basic_Init(gain);
		}
  }
	else if(gain>=16&&gain<=31) {
		  GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);
			GPIO_SetBits(GPIOD,GPIO_Pin_12);
		  GPIO_ResetBits(GPIOD,GPIO_Pin_13);
		  gain-=16;
	    if(gain>=0&&gain<=3)
				Basic_Init(gain);
      else if(gain>=4&&gain<=7) {
		    GPIO_ResetBits(GPIOC,GPIO_Pin_8);
				gain-=4;
				Basic_Init(gain);
      }
      else if(gain>=8&&gain<=15) {
				GPIO_ResetBits(GPIOD,GPIO_Pin_12);
				gain-=8;
				if(gain>=0&&gain<=3)
					Basic_Init(gain);
				else if(gain>=4&&gain<=7) {
					GPIO_ResetBits(GPIOC,GPIO_Pin_8);
					gain-=4;
					Basic_Init(gain);
				}
			}
  }
}
