#include "IIC.h"
#include "delay.h"
#include "main.h"

/**
*	函数说明：软件模拟 IIC
*
*	函数功能：GPIO 初始化
*
*	注意事项：GPIO 可自行选择
*
*	记录：	GPIO 引脚初始化,相同的归
*			属GPIOX尽量放同一语句中；
**/
void IIC_Init(void)
{		
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
//	   
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//SCL & SDA
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD ;   	//推挽/开漏
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		//速度 10 MHz
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 				//PB6,PB7 输出高
	
/*  已经在MX_GPIO_Init();中定义了*/
	  
}


/**
*	函数说明：产生IIC起始信号
*
*	原理说明：	当 SCL 是高电平状态时,
*				数据线 SDA 由高拉低;
**/
void IIC_Start(void)
{
    SDA_OUT();		/* SDA 线输出模式 */
	HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);	/* 先拉高 SDA 状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);	/* 再拉高 SCL 状态 */
    IIC_Delay();	/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);	/* 后拉低 SDA 状态 */
    IIC_Delay();	/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);	/* 钳住I2C,发送或接收 */
}



/**
*	函数说明：产生IIC停止信号
*
*	原理说明：	SCL线为高电平状态时, SDA 线
*				由低电平向高电平跳变(上升沿)
**/
void IIC_Stop(void)
{
    SDA_OUT();		/* SDA 线输出模式 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);	/* 先拉低 SCL 状态 */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);	/* 先拉低 SDA 状态 */
    IIC_Delay();	/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);	/* 后拉高 SCL 状态 */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);	/* 后拉高 SDA 状态 */
    IIC_Delay();	/* 延时保持电平状态 */
}



/**
*	函数说明：等待应答信号到来
*
*	原理说明：	先将数据线拉高,延时等待稳定,然后将时钟线拉高,
*				延时等待稳定,最后采样数据线电平状态,如果是高
*				电平,未应答,如果是低电平,应答;
*
*	返回值：0, 接收应答失败
*			1, 接收应答成功
**/
uint8_t IIC_Wait_Ack(void)
{
    uint16_t ErrorTime;
	
    SDA_IN();      			/** SDA 线输入模式 **/
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);			/* 先拉高 SDA 状态 */
    IIC_Delay();			/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);			/* 后拉高 SCL 状态 */
    IIC_Delay();			/* 延时保持电平状态 */
	
//    while(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))			/* 采样 SDA 电平状态 */
	while(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))			/* 采样 SDA 电平状态 */
    {
        ErrorTime ++;
		
        if(ErrorTime > 250)
        {
            IIC_Stop();		/* 非应答,结束通信 */
            return 0;
        }
    }
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);			/** 钳住,等待发或收 **/
	
    return 1;				/** 接收到应答信号 **/
}


/**
*	函数说明：产生 ACK 应答
*
*	原理说明：SCL 在高电平期间 SDA 始终处于低电平
*			 （SCL 保持时间 <= SDA 保持时间）
*			  需要在传输完毕一个字节后发送
*
**/
void IIC_Ack(void)
{
	SDA_OUT();					/** SDA 线输出模式 **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* 时钟线 SCL 拉低 */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);				/* 数据线 SDA 拉低 */
    IIC_Delay();				/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* 时钟线 SCL 拉高 */
    IIC_Delay();				/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* 时钟线 SCL 拉低 */
}


/**
*	函数说明：产生 NACK 应答
*
*	原理说明：SCL在高电平期间SDA始终处于高电平
*			 （SCL 保持时间 <= SDA 保持时间）
*			  需要在传输完毕一个字节后发送
*
**/
void IIC_NAck(void)
{
	SDA_OUT();					/** SDA 线输出模式 **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* 数据线 SCL 拉低 */
    HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);				/* 数据线 SDA 拉高 */
    IIC_Delay();				/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* 时钟线 SCL 拉高 */
    IIC_Delay();				/* 延时保持电平状态 */
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* 时钟线 SCL 拉低 */
}



/**
*	函数功能：IIC发送一个字节
*	参数说明：Txd	无符号 8 位
*	
*	返回值：1, 有应答
*			0, 无应答
**/
void IIC_Send_Byte(uint8_t Txd)
{
    uint8_t T;
	
    SDA_OUT();						/** 数据线SDA输出态 **/
    HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);					/** 时钟线 SCL 拉低 **/
	
    for(T = 0; T < 8; T++)			/** 由高到低发送数据 */
    {
//        IIC_SDA = (Txd & 0x80) >> 7;/** 每次发送最高位 **/
		if((Txd & 0x80) >> 7)	HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_SET);
		else	HAL_GPIO_WritePin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin,GPIO_PIN_RESET);
        Txd <<= 1;					/* 更新数据的最高位 */
        IIC_Delay();				/* 延时保持电平状态 */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);				/* 时钟线 SCL 拉高 */
        IIC_Delay();				/* 延时保持电平状态 */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);				/* 时钟线 SCL 拉低 */
        IIC_Delay();				/* 延时保持电平状态 */
    }
}


/*
 * 函数功能：IIC读字节
 * 参数说明：ack = 1, 发送ACK
 *			 ack = 0, 发送NACK
 *
 * 返回值：返回读取到的一个字节
 */
uint8_t IIC_Read_Byte(uint8_t ACK)
{
	uint8_t	T;
	uint8_t	Receive;

    SDA_IN();				/** SDA 线设置为输入 **/
	
    for(T = 0; T < 8; T++ )	/** 由高到低接收数据**/
    {
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_RESET);		/* 时钟线拉低的时候,SDA才允许变化 */
        IIC_Delay();		/* 延时保持电平状态 */
        HAL_GPIO_WritePin(BH1750_SCL_GPIO_Port,BH1750_SCL_Pin,GPIO_PIN_SET);		/* 拉高时钟线,不允许SDA变化,可以读取 SDA */
        Receive <<= 1;		/** 接收到数据位左移 **/
        if(HAL_GPIO_ReadPin(BH1750_SDA_GPIO_Port,BH1750_SDA_Pin))
        {
            Receive++;		/** 高电平,则最低位为1 **/
        }
        IIC_Delay();		/* 延时保持电平状态 */
    }
	
    if (!ACK)	
		IIC_NAck();			/** 发送 NACK **/
    else		
		IIC_Ack(); 			/** 发送  ACK **/
    return Receive;
}
