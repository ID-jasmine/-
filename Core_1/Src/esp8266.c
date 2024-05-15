/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	esp8266.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		ESP8266的简单驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f1xx_hal.h"

//网络设备驱动
#include "esp8266.h"

//硬件驱动

#include "usart.h"

//C库
#include "stdio.h"
#include "string.h"

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"yldrd\",\"11111111\"\r\n"           		//wifi name  wifi password      		
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"   	//代理端口			

unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
extern uint8_t rxBuf;


//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志

}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	HAL_UART_Transmit(&huart2,(unsigned char *)cmd,strlen((const char *)cmd), 1000);
	//Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				ESP8266_Clear();									//清空缓存
				
				return 0;
			}
		}
		
		HAL_Delay(10);
		//DelayXms(10);
	}
	
	return 1;

}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		HAL_UART_Transmit(&huart2,data,len, 1000);	
		//Usart_SendString(USART2, data, len);		//发送设备连接请求数据
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//搜索“IPD”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"\"IPD\" not found\r\n\r\n",strlen("\"IPD\" not found\r\n\r\n"), 1000);	
				//UsartPrintf(USART_DEBUG,"\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//找到':'
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}
		
		HAL_Delay(5);
		timeOut--;
		//DelayXms(5);													//延时等待
	} while(timeOut>0);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Init(void)
{
	
//  GPIO_InitTypeDef GPIO_Initure;
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//	//ESP8266复位引脚
//	GPIO_Initure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Initure.GPIO_Pin = GPIO_Pin_14;					//GPIOC14-复位
//	GPIO_Initure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_Initure);
//	
//	GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_RESET);
//	DelayXms(250);
//	GPIO_WriteBit(GPIOC, GPIO_Pin_14, Bit_SET);
//	DelayXms(500);
	
	ESP8266_Clear();
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"0. AT\r\n",sizeof("0. AT\r\n"), 1000);
	//UsartPrintf(USART_DEBUG, "1. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))HAL_Delay(500);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"1. RST\r\n",sizeof("1. RST\r\n"), 1000);
	ESP8266_SendCmd("AT+RST\r\n","");
	HAL_Delay(500);
	ESP8266_SendCmd("AT+CIPCLOSE\r\n","");   //关闭TCP连接，复位过程收不到返回下信息
	HAL_Delay(500);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"2. CWMODE\r\n",strlen( "2. CWMODE\r\n"), 1000);
	//UsartPrintf(USART_DEBUG, "2. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))HAL_Delay(500);	
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"3. AT+CWDHCP\r\n",strlen("3. AT+CWDHCP\r\n"), 1000);	
	//UsartPrintf(USART_DEBUG, "3. AT+CWDHCP\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))HAL_Delay(500);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"4. CWJAP\r\n",strlen("4. CWJAP\r\n"), 1000);	
	//UsartPrintf(USART_DEBUG, "4. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))HAL_Delay(500);
	
	/*一条设置单路连接的命令*/
	HAL_UART_Transmit(&huart1,(uint8_t *)"5. CWMODE\r\n",strlen("5. CWMODE\r\n"),1000);
	while(ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"))
	HAL_Delay(500);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"6. CIPSTART\r\n",strlen("6. CIPSTART\r\n"), 1000);	
	//UsartPrintf(USART_DEBUG, "5. CIPSTART\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))HAL_Delay(500);
	
	HAL_UART_Transmit(&huart1,(uint8_t *)"7. ESP8266 Init OK\r\n",strlen( "7. ESP8266 Init OK\r\n"), 1000);
	//UsartPrintf(USART_DEBUG, "6. ESP8266 Init OK\r\n");

}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
//void USART2_IRQHandler(void)
//{

//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
//	{
//		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
//		esp8266_buf[esp8266_cnt++] = USART2->DR;
//		
//		USART_ClearFlag(USART2, USART_FLAG_RXNE);
//	}

//}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance==USART2){
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = rxBuf;
		
		HAL_UART_Receive_IT(&huart2,&rxBuf,1);
	}
}
