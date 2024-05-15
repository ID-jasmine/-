/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	esp8266.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		ESP8266�ļ�����
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f1xx_hal.h"

//�����豸����
#include "esp8266.h"

//Ӳ������

#include "usart.h"

//C��
#include "stdio.h"
#include "string.h"

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"yldrd\",\"11111111\"\r\n"           		//wifi name  wifi password      		
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"broker.emqx.io\",1883\r\n"   	//����˿�			

unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
extern uint8_t rxBuf;


//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־

}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	HAL_UART_Transmit(&huart2,(unsigned char *)cmd,strlen((const char *)cmd), 1000);
	//Usart_SendString(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				ESP8266_Clear();									//��ջ���
				
				return 0;
			}
		}
		
		HAL_Delay(10);
		//DelayXms(10);
	}
	
	return 1;

}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();								//��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		HAL_UART_Transmit(&huart2,data,len, 1000);	
		//Usart_SendString(USART2, data, len);		//�����豸������������
	}

}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//������IPD��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"\"IPD\" not found\r\n\r\n",strlen("\"IPD\" not found\r\n\r\n"), 1000);	
				//UsartPrintf(USART_DEBUG,"\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');							//�ҵ�':'
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
		//DelayXms(5);													//��ʱ�ȴ�
	} while(timeOut>0);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Init(void)
{
	
//  GPIO_InitTypeDef GPIO_Initure;
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//	//ESP8266��λ����
//	GPIO_Initure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Initure.GPIO_Pin = GPIO_Pin_14;					//GPIOC14-��λ
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
	ESP8266_SendCmd("AT+CIPCLOSE\r\n","");   //�ر�TCP���ӣ���λ�����ղ�����������Ϣ
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
	
	/*һ�����õ�·���ӵ�����*/
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
//	�������ƣ�	USART2_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
//void USART2_IRQHandler(void)
//{

//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //�����ж�
//	{
//		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
//		esp8266_buf[esp8266_cnt++] = USART2->DR;
//		
//		USART_ClearFlag(USART2, USART_FLAG_RXNE);
//	}

//}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance==USART2){
		if(esp8266_cnt >= sizeof(esp8266_buf))	esp8266_cnt = 0; //��ֹ���ڱ�ˢ��
		esp8266_buf[esp8266_cnt++] = rxBuf;
		
		HAL_UART_Receive_IT(&huart2,&rxBuf,1);
	}
}
