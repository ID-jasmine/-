/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.1
	*
	*	说明： 		与onenet平台的数据交互接口层
	*
	*	修改记录：	V1.0：协议封装、返回判断都在同一个文件，并且不同协议接口不同。
	*				V1.1：提供统一接口供应用层使用，根据不同协议文件来封装协议相关的内容。
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
//#include "stm32f1xx_hal.h"
#include "main.h"

//网络设备
#include "esp8266.h"
 
//协议文件
#include "onenet.h"
#include "mqttkit.h"

//硬件驱动
#include "usart.h"
//cJSON
#include "cjson.h"

//C库
#include <string.h>
#include <stdio.h>



#define PROID		"77249"    		//"77247"
//产品名	
#define AUTH_INFO	"test"

#define DEVID		"5616831"		//"5616839"
//设备

extern unsigned char esp8266_buf[128];
char temp[70];
cJSON *json,*json_value;

extern uint8_t marks_LED,marks_BEEP,alarm_free;
uint8_t VAL_CMD=0;
char TAR_CMD[20];

//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	
	_Bool status = 1;
	
	memset(temp,0,70);
	sprintf(temp,"OneNet_DevLink\r\n"
				"PROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
	HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp),1000);
	//UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
	//"PROID: %s,	AUIF: %s,	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);
	
	if(MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:
						HAL_UART_Transmit(&huart1,(uint8_t *) "Tips:连接成功\r\n",strlen( "Tips:连接成功\r\n"), 1000);
						status = 0;
					break;
						//UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n")
					
					case 1:
						HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:连接失败：协议错误\r\n",strlen( "WARN:	连接失败：协议错误\r\n"), 1000);
					break;
					//UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");
					case 2:HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:	连接失败：非法的clientid\r\n",strlen( "WARN:		连接失败：非法的clientid\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法的clientid\r\n");
					case 3:HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:	连接失败：服务器失败\r\n",strlen( "WARN:	连接失败： 	服务器失败\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");
					case 4:HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:	连接失败：用户名或密码错误\r\n",strlen("WARN:	连接失败：用户名或密码错误\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户名或密码错误\r\n");
					case 5:HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:	连接失败：非法链接(比如token非法)\r\n",strlen("WARN:		连接失败：非法链接(比如token非法)\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法链接(比如token非法)\r\n");
					
					default:HAL_UART_Transmit(&huart1,(uint8_t *)"ERR:	连接失败：未知错误\r\n",strlen("ERR:		连接失败：未知错误\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");
				}
			}
		}
		else
		{
			memset(temp,0,70);
			sprintf(temp,"dataPtr == NULL\r\n");
			HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else	HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:MQTT_PacketConnect Failed\r\n",strlen("WARN:MQTT_PacketConnect Failed\r\n"), 1000);
		//UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");

	return status;
	
}

//==========================================================
//	函数名称：	OneNet_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
	for(; i < topic_cnt; i++){
		memset(temp,0,70);
		sprintf(temp,"Subscribe Topic: %s\r\n", topics[i]);
		HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
	}
		//UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}

}

//==========================================================
//	函数名称：	OneNet_Publish
//
//	函数功能：	发布消息
//
//	入口参数：	topic：发布的主题
//				msg：消息内容
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_PUBLISH-需要重送
//
//	说明：		
//==========================================================
void OneNet_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
	memset(temp,0,70);
	sprintf(temp,"Publish Topic: %s, Msg: %s\r\n", topic, msg);
	HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
	//UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}

}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	memset(TAR_CMD,0,20);
	switch(type)
	{
		case MQTT_PKT_CMD:															//命令下发
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
			if(result == 0)
			{
				memset(temp,0,70);
				sprintf(temp,"cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
				//UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				{
					HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send CmdResp\r\n",strlen("Tips:	Send CmdResp\r\n"), 1000);
					//UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令
					MQTT_DeleteBuffer(&mqttPacket);									//删包
				}
			}
		
		break;
		
		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				memset(temp,0,70);
				sprintf(temp,"topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
				//UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
				//												cmdid_topic, topic_len, req_payload, req_len);
				//对数据包req_payload进行JSON格式解析
				json = cJSON_Parse(req_payload);
				if(!json)//无数据发送错误报告
				{
					memset(temp,0,70);
					sprintf(temp,"Error before:[%s]\r\n",cJSON_GetErrorPtr());
					HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
				}else{
					VAL_CMD=Json_getValue(req_payload);
					memset(TAR_CMD,0,20);
					Json_getTarget(req_payload,TAR_CMD);
					//json_value = cJSON_GetObjectItem(json,"LED");json_value->
					memset(temp,0,70);
					sprintf(temp,"json_value=%d\r\n\r\n",VAL_CMD);
					HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
					if(!abs(strcmp(TAR_CMD,"LED"))){
						if(VAL_CMD==1)
						{
							LED_1;
							marks_LED=1;
							alarm_free=0;
						}else if(VAL_CMD==0){
							LED_0;
							marks_LED=0;
							alarm_free=0;
						}
					}
					if(!abs(strcmp(TAR_CMD,"BEEP"))){
						if(VAL_CMD==1)
						{
							BEEP_1;
							marks_BEEP=1;
							alarm_free=0;
						}else if(VAL_CMD==0){
							BEEP_0;
							marks_BEEP=0;
							alarm_free=0;
						}
					}
					
				}
				cJSON_Delete(json);//删包，不然爆炸
			}
		
		break;
//		case MQTT_PKT_PUBLISH:														//接收的Publish消息
//		
//			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
//			if(result == 0)
//			{
//				memset(temp,0,70);
//				sprintf(temp,"topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
//																	cmdid_topic, topic_len, req_payload, req_len);
//				HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
//				//UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
//				//												cmdid_topic, topic_len, req_payload, req_len);
//				
//				switch(qos)
//				{
//					case 1:															//收到publish的qos为1，设备需要回复Ack
//					
//						if(MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
//						{
//							HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send PublishAck\r\n",strlen("Tips:	Send PublishAck\r\n"), 1000);
//							//UsartPrintf(USART_DEBUG, "Tips:	Send PublishAck\r\n");
//							ESP8266_SendData(mqttPacket._data, mqttPacket._len);
//							MQTT_DeleteBuffer(&mqttPacket);
//						}
//					
//					break;
//					
//					case 2:															//收到publish的qos为2，设备先回复Rec
//																					//平台回复Rel，设备再回复Comp
//						if(MQTT_PacketPublishRec(pkt_id, &mqttPacket) == 0)
//						{
//							HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send PublishRec\r\n",strlen("Tips:	Send PublishRec\r\n"), 1000);
//							//UsartPrintf(USART_DEBUG, "Tips:	Send PublishRec\r\n");
//							ESP8266_SendData(mqttPacket._data, mqttPacket._len);
//							MQTT_DeleteBuffer(&mqttPacket);
//						}
//					
//					break;
//					
//					default:
//						break;
//				}
//			}
//		
//		break;
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Publish Send OK\r\n",strlen("Tips:	MQTT Publish Send OK\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
		break;
			
		case MQTT_PKT_PUBREC:														//发送Publish消息，平台回复的Rec，设备需回复Rel消息
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Rev PublishRec\r\n",strlen("Tips:	Rev PublishRec\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send PublishRel\r\n",strlen("Tips:	Send PublishRel\r\n"), 1000);
					//UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:														//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Rev PublishRel\r\n",strlen("Tips:	Rev PublishRel\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send PublishComp\r\n",strlen("Tips:	Send PublishComp\r\n"), 1000);
					//UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:														//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Rev PublishComp\r\n",strlen("Tips:	Rev PublishComp\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//发送Subscribe消息的Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Subscribe OK\r\n",strlen("Tips:	MQTT Subscribe OK\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Subscribe Err\r\n",strlen("Tips:	MQTT Subscribe Err\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:														//发送UnSubscribe消息的Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT UnSubscribe OK\r\n",strlen("Tips:	MQTT UnSubscribe OK\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
			else
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT UnSubscribe Err\r\n",strlen("Tips:	MQTT UnSubscribe Err\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//清空缓存
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '}');					//搜索'}'

	if(dataPtr != NULL && result != -1)					//如果找到了
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//判断是否是下发的命令控制数据
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//转为数值形式
		
	}

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
