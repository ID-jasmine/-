#include "BH1750GY302_1.h"
#include "IIC.h"


/****************************************************************************
**	函数名称：BH1750_WriteByte()
**	函数功能：BH1750 写数据
**	参数说明：
**			uint8_t addr: 寄存器地址
**
**	返回值：无
**	IIC函数说明：
**				I2C_GetFlagStatus（IIC_x, IIC_Flag_x）-- 检查标志位是否置 1
**
*****************************************************************************/
void BH1750_WriteByte(uint8_t addr)
{	
	IIC_Start();								// 1.发送起始信号
	
	IIC_Send_Byte(BH1750_Addr_NOW_REG << 1|0);	// 2.发送7位地址与读写位（0：写）
	while(!IIC_Wait_Ack());						// 3.等待从机应答信号
	
	IIC_Send_Byte(addr);						// 4.发送8位操作指令
	while(!IIC_Wait_Ack());						// 5.等待从机应答信号
	
	IIC_Stop();									// 6.发送停止信号
}



/****************************************************************************
**	函数名称：BH1750_Read_Data()
**	函数功能：BH1750 读取数据
**	参数说明：
**			uint8_t addr ：寄存器地址
**
**	返回值：RX_Data
**	IIC函数说明：
**				I2C_GetFlagStatus（IIC_x, IIC_Flag_x）-- 检查标志位是否置 1
**
*****************************************************************************/
uint16_t BH1750_Read_Data(void)
{
    uint8_t 	H_Data;							//高 8 位数据
	uint8_t 	L_Data;							//低 8 位 数据
	uint16_t 	Rx_Data;						//完整 16 位数据
	
	IIC_Start();								// 1.发送起始信号
	IIC_Send_Byte(BH1750_Addr_NOW_REG << 1|1);	// 2.发送7位地址与读写位（1：读）
	while(!IIC_Wait_Ack());						// 3.等待从机应答信号
	
	H_Data = IIC_Read_Byte(1);					// 4.读取高八位并发送ACK
	L_Data = IIC_Read_Byte(0);					// 5.读取低八位并发送NACK
	
	IIC_Stop();									// 6.发送停止信号
    Rx_Data = ((uint16_t)H_Data << 8) + L_Data;	// 7.16位数据合成
	
    return Rx_Data;								// 8.返回读取数据
}


/**	BH1750上电 **/
void BH1750_Power_ON(void)
{
	BH1750_WriteByte(BH1750_Power_ON_REG);
}

/**	BH1750断电 **/
void BH1750_Power_OFF(void)
{
	BH1750_WriteByte(BH1750_Power_OFF_REG);
}

/**	BH1750数据寄存器复位 **/
void BH1750_RESET(void)
{
	BH1750_WriteByte(BH1750_MODULE_RESET_REG);
}

/**	BH1750初始化 **/
void BH1750_Init(void)
{
	IIC_Init();
    HAL_Delay(100);
	BH1750_Power_OFF();
	BH1750_Power_ON();
	BH1750_RESET();
	BH1750_WriteByte(BH1750_CONTINUE_H_MODE2);
}

/**	
**	获取光照强度
**	分辨率	光照强度（单位lx）=（High Byte + Low Byte）/ 1.2  
**/
float Light_Intensity(void)
{
	return (float)(BH1750_Read_Data() / 1.2f );
}

/*************************************END***********************************************/
