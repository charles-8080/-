/*********************************************************************************
**********************************************************************************
* 文件名称: w25qxx.c                                                            	 *
* 文件简述：W25Q128使用                                                         	 *
* 创建日期：2017.08.30                                                          	 *
* 说    明：25Q128容量为16M,共有128个Block,4096个Sector                         	 *
            255byte为一页                                                       	 *
						4Kbytes为一个扇区                                                   	 *
            16个扇区为一个块                                                    	 *
**********************************************************************************
*********************************************************************************/

#include "w25qxx.h"
#include "spi.h"

u16 W25QXX_ID;
u8 pBuffer[Byte_Count] = {7,13};	//初始值设定
u8 DataBuffer[Byte_Count];	//读取数据保存

//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{ 
	W25QXX_CS=1;			                                    //SPI FLASH不选中
	SPI3_Init();		   			                              //初始化SPI
	W25QXX_ID=W25QXX_ReadID();	                          //读取FLASH ID.
}

/****************************************************************************
* 名    称: u8 W25QXX_ReadSR(void) 
* 功    能：读取W25QXX的状态寄存器
* 入口参数：无
* 返回参数：状态寄存器的值
* 说    明： 		     
****************************************************************************/
u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	W25QXX_CS=0;                                  //使能器件   
	SPI3_ReadWriteByte(W25X_CMD_ReadStatusReg);   //发送读取状态寄存器命令    
	byte=SPI3_ReadWriteByte(0Xff);                //读取一个字节  
	W25QXX_CS=1;                                  //取消片选     
	return byte;   
} 

/****************************************************************************
* 名    称: void W25QXX_Write_SR(u8 sr)
* 功    能：写W25QXX状态寄存器
* 入口参数：写入的值
* 返回参数：无
* 说    明： 		     
****************************************************************************/
void W25QXX_Write_SR(u8 sr)   
{   
	W25QXX_CS=0;                                 //使能器件   
	SPI3_ReadWriteByte(W25X_CMD_WriteStatusReg); //发送写取状态寄存器命令    
	SPI3_ReadWriteByte(sr);                      //写入一个字节  
	W25QXX_CS=1;                                 //取消片选     	      
} 

//W25QXX写使能	 
void W25QXX_Write_Enable(void)   
{
	W25QXX_CS=0;                               //使能器件   
  SPI3_ReadWriteByte(W25X_CMD_WriteEnable);  //发送写使能  
	W25QXX_CS=1;                               //取消片选     	      
} 

//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);      //等待BUSY位清空
} 

//W25QXX写禁止	 
void W25QXX_Write_Disable(void)   
{  
	W25QXX_CS=0;                                //使能器件   
  SPI3_ReadWriteByte(W25X_CMD_WriteDisable);  //发送写禁止指令    
	W25QXX_CS=1;                                //取消片选     	      
} 

/****************************************************************************
* 名    称: u16 W25QXX_ReadID(void)
* 功    能：读取芯片ID
* 入口参数：无
* 返回参数：芯片ID
* 说    明：0XEF13,表示芯片型号为W25Q80  
            0XEF14,表示芯片型号为W25Q16    
            0XEF15,表示芯片型号为W25Q32  
            0XEF16,表示芯片型号为W25Q64 
            0XEF17,表示芯片型号为W25Q128      
****************************************************************************/  
u16 W25QXX_ReadID(void)
{
	u16 IDnum = 0;	  
	W25QXX_CS=0;				    
	SPI3_ReadWriteByte(0x90); //发送读取ID命令	    
	SPI3_ReadWriteByte(0x00); 	    
	SPI3_ReadWriteByte(0x00); 	    
	SPI3_ReadWriteByte(0x00); 	 			   
	IDnum|=SPI3_ReadWriteByte(0xFF)<<8;  
	IDnum|=SPI3_ReadWriteByte(0xFF);	 
	W25QXX_CS=1;				    
	return IDnum;
} 

/****************************************************************************
* 名    称: void W25QXX_Erase_Chip(void) 
* 功    能：擦除整个芯片		  
* 入口参数：无
* 返回参数：无
* 说    明： 		     
****************************************************************************/
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                   
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                             //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_ChipErase);  //发送片擦除命令  
	  W25QXX_CS=1;                             //取消片选     	      
	  W25QXX_Wait_Busy();   				           //等待芯片擦除结束
}  

/****************************************************************************
* 名    称: void W25QXX_Erase_Sector(u32 First_Addr)
* 功    能：擦除某个扇区的首地址	  
* 入口参数：First_Addr:扇区地址
* 返回参数：无
* 说    明： 		     
****************************************************************************/
void W25QXX_Erase_Sector(u32 First_Addr)   
{    
 	  First_Addr*=4096;
    W25QXX_Write_Enable();                  
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                                  //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_SectorErase);     //发送扇区擦除指令 
    SPI3_ReadWriteByte((u8)((First_Addr)>>16));   //发送地址    
    SPI3_ReadWriteByte((u8)((First_Addr)>>8));   
    SPI3_ReadWriteByte((u8)First_Addr);  
	  W25QXX_CS=1;                                  //取消片选     	      
    W25QXX_Wait_Busy();   				                //等待擦除完成
} 

//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                             //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_PowerDown);  //发送掉电命令  
	  W25QXX_CS=1;                             //取消片选     	      
    delay_us(3);                             
}  

//唤醒
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                                      //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_ReleasePowerDown);    //发送唤醒命令
	  W25QXX_CS=1;                                      //取消片选     	      
    delay_us(3);                                     
}   

/****************************************************************************
* 名    称: void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)
* 功    能：指定地址开始读取指定长度的数据
* 入口参数：DataBuffer:数据存储区
            StartAddress:开始读取的地址(24bit)
            ByteCount:要读取的字节数(最大65535)
* 返回参数：无
* 说    明： 		     
****************************************************************************/
void W25QXX_Read(u8* DataBuffer,u32 StartAddress,u16 ByteCount)   
{  										    
	W25QXX_CS=0;                                 //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_ReadData);     //发送读取命令   
    SPI3_ReadWriteByte((u8)((StartAddress)>>16));  //发送24bit地址    
    SPI3_ReadWriteByte((u8)((StartAddress)>>8));   
    SPI3_ReadWriteByte((u8)StartAddress);   
	
			while (ByteCount--) 
			{
					*DataBuffer = SPI3_ReadWriteByte(0XFF);
					DataBuffer++;
			}		
	W25QXX_CS=1;  				    	      
}  

/****************************************************************************
* 名    称: void W25QXX_Write_Page(u8* DataBuffer,u32 StartAddress,u16 ByteCount)
* 功    能：在一页内写入少于256个字节的数据  
* 入口参数：DataBuffer:数据存储区
            StartAddress:开始写入的地址(24bit)
            ByteCount:要写入的字节数(最大256)
* 返回参数：无
* 说    明： 		     
****************************************************************************/
void W25QXX_Write_Page(u8* DataBuffer,u32 StartAddress,u16 ByteCount)
{  
	W25QXX_Write_Enable();                   
	  W25QXX_CS=0;                                   //使能器件   
    SPI3_ReadWriteByte(W25X_CMD_PageProgram);      //发送写页命令   
    SPI3_ReadWriteByte((u8)((StartAddress)>>16));  //发送地址    
    SPI3_ReadWriteByte((u8)((StartAddress)>>8));   
    SPI3_ReadWriteByte((u8)StartAddress);   
		while (ByteCount--)
			{
			  SPI3_ReadWriteByte(*DataBuffer);
			  DataBuffer++;
			}	
	W25QXX_CS=1;                   //取消片选 
	W25QXX_Wait_Busy();					   //等待写入结束
}

/****************************************************************************
* 名    称: void W25QXX_PageWrite(u8* DataBuffer,u32 StartAddress,u16 ByteCount) 
* 功    能：按页写入数据
* 入口参数：DataBuffer:数据存储区
            StartAddress:开始写入的地址(24bit)
            ByteCount:要写入的字节数(最大256)
* 返回参数：无
* 说    明：有自动换页功能，因为该函数没有在写之前判断要写入的地址上的数据是否为
            空，所以在写入之前必须确保该地址上的数据为空0xFF，否则写入失败 		     
****************************************************************************/
void W25QXX_PageWrite(u8* DataBuffer,u32 StartAddress,u16 ByteCount)   
{
  u8 NumOfPage = 0, NumOfSingle = 0, Surplus_Addr = 0, Surplus_count = 0, midtemp = 0;

  Surplus_Addr = StartAddress % 256;
  Surplus_count = 256 - Surplus_Addr;
  NumOfPage =  ByteCount / 256;
  NumOfSingle = ByteCount % 256;

  if (Surplus_Addr == 0) //开始写的地址刚好是页开始的地址
  {
    if (NumOfPage == 0)  // ByteCount < 256（一页总的字节数）  
    {
      W25QXX_Write_Page(DataBuffer, StartAddress, ByteCount);
    }
    else                 // ByteCount > 256（一页总的字节数）  
    {
      while (NumOfPage--)
      {
        W25QXX_Write_Page(DataBuffer, StartAddress, 256);
        StartAddress +=  256;
        DataBuffer += 256;
      }
      W25QXX_Write_Page(DataBuffer, StartAddress, NumOfSingle);
    }
  }
  else ///开始写的地址不在页的首地址上
  {
    if (NumOfPage == 0) // ByteCount < 256（一页总的字节数） 
    {
      if (NumOfSingle > Surplus_count)  
      {
        midtemp = NumOfSingle - Surplus_count;
        W25QXX_Write_Page(DataBuffer, StartAddress, Surplus_count);
        StartAddress +=  Surplus_count;
        DataBuffer += Surplus_count;
        W25QXX_Write_Page(DataBuffer, StartAddress, midtemp);
      }
      else
      {
        W25QXX_Write_Page(DataBuffer, StartAddress, ByteCount);
      }
    }
    else //ByteCount > 256（一页总的字节数）  
    {
      ByteCount -= Surplus_count;
      NumOfPage =  ByteCount / 256;
      NumOfSingle = ByteCount % 256;

      W25QXX_Write_Page(DataBuffer, StartAddress, Surplus_count);
      StartAddress +=  Surplus_count;
      DataBuffer += Surplus_count;
      while (NumOfPage--)
      {
        W25QXX_Write_Page(DataBuffer, StartAddress, 256);
        StartAddress +=  256;
        DataBuffer += 256;
      }
      if (NumOfSingle != 0)
      {
        W25QXX_Write_Page(DataBuffer, StartAddress, NumOfSingle);
      }
    }
  }
} 

/****************************************************************************
* 名    称: void W25QXX_SectorWrite(u8* DataBuffer,u32 StartAddress,u16 ByteCount)
* 功    能：按扇区写入数据
* 入口参数：DataBuffer:数据存储区
            StartAddress:开始写入的地址(24bit)
            ByteCount:要写入的字节数(最大65536)
* 返回参数：无
* 说    明：写入之前判断改地址上的数据是否为空，不是先擦除再写入，该函数参考网上代码http://www.openedv.com      
****************************************************************************/
u8 TS_BUFFER[4096];
void W25QXX_SectorWrite(u8* DataBuffer,u32 StartAddress,u16 ByteCount)   
{ 
	u32 secaddr;
	u16 secused;
	u16 Surplus_count;	   
 	u16 i=0; 
	
	u8 * Cache_BUF;	   
  Cache_BUF=TS_BUFFER;	  
	
 	secaddr=StartAddress/4096;//扇区地址  
	secused=StartAddress%4096;//写入的地址在扇区内的偏移（从该扇区的首地址开始已使用的字节数）
	Surplus_count=4096-secused;//该扇区剩余空间大小   

 	if(ByteCount<=Surplus_count)Surplus_count=ByteCount;//不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(Cache_BUF,secaddr*4096,4096);//读出整个扇区的内容
		      i=0;
		while(Cache_BUF[secused+i]==0XFF)
				{
					i++;
					if(i==Surplus_count)break;
				}
		if(i<Surplus_count)//需要擦除
				{
					W25QXX_Erase_Sector(secaddr);//擦除这个扇区
					for(i=0;i<Surplus_count;i++)	   //复制
					{
						Cache_BUF[i+secused]=DataBuffer[i];	  
					}
					W25QXX_PageWrite(Cache_BUF,secaddr*4096,4096);//写入整个扇区  

				}
		else
			  W25QXX_PageWrite(DataBuffer,StartAddress,Surplus_count);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(ByteCount==Surplus_count)break;//写入结束了
		else//写入未结束
				{
					secaddr++;//扇区地址增1
					secused=0;//偏移位置为0 	 

					DataBuffer+=Surplus_count;  //指针偏移
					StartAddress+=Surplus_count;//写地址偏移	   
					ByteCount-=Surplus_count;				//字节数递减
					if(ByteCount>4096)Surplus_count=4096;	//下一个扇区还是写不完
					else Surplus_count=ByteCount;			//下一个扇区可以写完了
				}	 
	};
}
