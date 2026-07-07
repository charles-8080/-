#include "ad9959.h"

//RESET  PC13
//SDIO0  PE0
//SDIO1  PE1
//SDIO2  PE2
//SDIO3  PE3
//CS     PE4
//SCLK   PE5
//UPDATE PE6
//其余引脚接地
//----------------------------引脚接口如上----------------------------
uint8_t FR1_DATA[3] = {0xD0,0x00,0x00};	//20倍频 Charge pump control = 75uA FR1<23> -- VCO gain control =0时 system clock below 160 MHz;
uint8_t FR2_DATA[2] = {0x20,0x00};
uint8_t CFR_DATA[3] = {0x00,0x03,0x02};

uint8_t CFTW0_DATA[4] = {0x00,0x00,0x00,0x00};
uint8_t CPOW0_DATA[2] = {0x00,0x00};
uint8_t ACR_DATA[3] = {0x00,0x00,0x00};

uint8_t LSRR_DATA[2] = {0x00,0x00};
uint8_t RDW_DATA[4] = {0x00,0x00,0x00,0x00};
uint8_t FDW_DATA[4] = {0x00,0x00,0x00,0x00};
//----------------------------上面为寄存器设定值----------------------
																																		 
//----------------------------上面暂时不要更改------------------------
//----------------------------下面为函数声明--------------------------

//AD9959更新IO寄存器
void AD9959_IO_update(void) {
	AD9959_UPDATE = 0;
	delay_us(2);
	AD9959_UPDATE = 1;
	delay_us(4);
	AD9959_UPDATE = 0;
}

/**
* @brief 通过SPI向AD9959写数据
* @param register_address 寄存器地址
* @param number_of_registers 所含字节数
* @param register_data 数据起始地址
* @param update 是否更新IO寄存器
**/
void AD9959_write_data(AD9959_REG_ADDR register_address, uint8_t number_of_registers, const uint8_t *register_data, bool update)
{
	uint8_t ControlValue = 0;
	uint8_t ValueToWrite = 0;
	uint8_t RegisterIndex = 0;
	uint8_t i = 0;

	assert_param(IS_AD9959_REG_ADDR(register_address));
	ControlValue = register_address;
	
	//写入地址
	AD9959_SCLK = 0;
	AD9959_CS = 0;
	for(i=0;i<8;i++) {
		AD9959_SCLK = 0;
		if (0x80==(ControlValue&0x80))
			AD9959_SDIO0 = 1;
		else
			AD9959_SDIO0 = 0;
		AD9959_SCLK = 1;
		ControlValue <<= 1;
	}
	AD9959_SCLK = 0;
	
	//写入数据
	for (RegisterIndex = 0; RegisterIndex < number_of_registers; RegisterIndex++) {
		ValueToWrite = register_data[RegisterIndex];
		for(i=0;i<8;i++) {
			AD9959_SCLK = 0;
			if (0x80==(ValueToWrite&0x80))
				AD9959_SDIO0 = 1;
			else
				AD9959_SDIO0 = 0;
			AD9959_SCLK = 1;
			ValueToWrite <<= 1;
		}
		AD9959_SCLK = 0;
	}
	if (update)
		AD9959_IO_update();
	AD9959_CS = 1;
}

//AD9959初始化
void AD9959_Init(void)
{
	//使能GPIO时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOE, ENABLE);
	//GPIO初始化
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	AD9959_CS = 1;
	AD9959_SCLK = 0;
	AD9959_UPDATE = 0;
	AD9959_SDIO0 = 0;
	AD9959_SDIO1 = 0;
	AD9959_SDIO2 = 0;
	AD9959_SDIO3 = 0;
	
	AD9959_RESET = 0;
	delay_us(1);
	AD9959_RESET = 1;
	delay_us(30);
	AD9959_RESET = 0;

	//写功能寄存器1
	AD9959_write_data(AD9959_REG_FR1,3,FR1_DATA,1);
	AD9959_write_data(AD9959_REG_FR2,2,FR2_DATA,1);

	//写入初始频率
	AD9959_write_frequency(AD9959_CHANNEL_0,10000);
	AD9959_write_frequency(AD9959_CHANNEL_1,10000);
	AD9959_write_frequency(AD9959_CHANNEL_2,10000);
	AD9959_write_frequency(AD9959_CHANNEL_3,10000);
	
	//写入初始相位
	AD9959_write_phase(AD9959_CHANNEL_0,0);
	AD9959_write_phase(AD9959_CHANNEL_1,45);
	AD9959_write_phase(AD9959_CHANNEL_2,90);
	AD9959_write_phase(AD9959_CHANNEL_3,135);
	
	////写入初始幅度
	AD9959_write_amplitude(AD9959_CHANNEL_0,0x3FF);
	AD9959_write_amplitude(AD9959_CHANNEL_1,0x3FF);
	AD9959_write_amplitude(AD9959_CHANNEL_2,0x3FF);
	AD9959_write_amplitude(AD9959_CHANNEL_3,0x3FF);
}

/**
* @brief 设置通道输出频率
* @param channel 输出通道
* @param Freq 输出频率 (频率范围 1~200,000,000Hz)
**/
void AD9959_write_frequency(AD9959_CHANNEL channel, uint32_t Freq)
{
	uint8_t cs_data = channel;
	assert_param(IS_AD9959_CHANNEL(channel));

	uint32_t frequency;
	frequency = (uint32_t) Freq * 8.589934592;	//将输入频率因子分为四个字节  8.589934592=(2^32)/500000000 其中500M=25M*20(倍频数可编程)
	CFTW0_DATA[3] = (uint8_t)frequency;
	CFTW0_DATA[2] = (uint8_t)(frequency>>8);
	CFTW0_DATA[1] = (uint8_t)(frequency>>16);
	CFTW0_DATA[0] = (uint8_t)(frequency>>24);

	AD9959_write_data(AD9959_REG_CSR,1,&cs_data,1);
	AD9959_write_data(AD9959_REG_CFTW0,4,CFTW0_DATA,1);
}

/**
* @brief 设置通道输出相位
* @param channel 输出通道
* @param phase 输出相位范围：0~360°,14bit
**/
void AD9959_write_phase(AD9959_CHANNEL channel, uint16_t Pha)
{
	uint8_t cs_data = channel;
	assert_param(IS_AD9959_CHANNEL(channel));
	
	uint16_t phase;
	phase = (uint16_t) Pha * 45.511111;	//将输入相位差写入，进度1度，45.511111=2^14/360
	CPOW0_DATA[1] = (uint8_t)phase;
	CPOW0_DATA[0] = (uint8_t)(phase>>8);
	
	AD9959_write_data(AD9959_REG_CSR,1,&cs_data,1);
	AD9959_write_data(AD9959_REG_CPOW0,2,CPOW0_DATA,1);
}

/**
* @brief 设置通道输出幅度
* @param channel 输出通道
* @param amplitude 输出幅度范围：0~1023(对应幅度：0~530mV),10bit
**/
void AD9959_write_amplitude(AD9959_CHANNEL channel, uint16_t amplitude)
{
	uint8_t cs_data = channel;
	assert_param(IS_AD9959_CHANNEL(channel));

	amplitude |= 0x1000;	//幅度范围：0-1023，对应幅度：0-530mV
	ACR_DATA[2] = (uint8_t)amplitude;
	ACR_DATA[1] = (uint8_t)(amplitude>>8);

	AD9959_write_data(AD9959_REG_CSR,1,&cs_data,1);
	AD9959_write_data(AD9959_REG_ACR,3,ACR_DATA,1);
}
