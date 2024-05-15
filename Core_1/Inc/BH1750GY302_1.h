#ifndef __BH1750GY302_1_H
#define __BH1750GY302_1_H


#include "main.h"
#include "delay.h"





/*	器件地址(7位-用左移1位-最后1位为读写位--1为读-0为写) */
#define				BH1750_Addr_GND_REG				0X23	//ADDR引脚接低电平
#define				BH1750_Addr_VCC_REG				0X5C	//ADDR引脚接高电平

#define				BH1750_Addr_NOW_REG				BH1750_Addr_GND_REG	//ADDR引脚

				/******	指令集宏定义*****/
#define				BH1750_Power_OFF_REG			0x00	//断电指令
#define				BH1750_Power_ON_REG				0x01	//通电,等待测量命令
#define				BH1750_MODULE_RESET_REG			0x07	//重置数据寄存器,关机模式下，不能接受重置命令


/*************************************************	
	不同模式下分辨率不同（也即精度不同）
	高分辨率模式2：分辨率是0.5lx
	高分辨率模式：分辨率1lx
	低分辨率模式：分辨率4lx
	不同模式只是精度不一样，对于计算没有区别
***************************************************/
	/**	工作模式指令集	**/
#define				BH1750_CONTINUE_H_MODE			0x10	//连续H分辨率模式
#define				BH1750_CONTINUE_H_MODE2			0x11	//连续H分辨率模式2
#define				BH1750_CONTINUE_L_MODE			0x13	//连续L分辨率模式
#define				BH1750_ONE_TIME_H_MODE			0x20	//一次H分辨率模式
#define				BH1750_ONE_TIME_H_MODE2			0x21	//一次H分辨率模式2
#define				BH1750_ONE_TIME_L_MODE			0x23	//一次性L分辨率模式



/**		函数声明区	**/
void 		I2C_Configuration				(void);
void 		BH1750_WriteByte				(uint8_t addr);
uint16_t 	BH1750_Read_Data				(void);
void 		BH1750_Power_ON					(void);
void 		BH1750_Power_OFF				(void);
void 		BH1750_RESET					(void);
void 		BH1750_Init						(void);
float 		Light_Intensity					(void);
		

	
#endif

