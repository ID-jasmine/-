/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
	*
	*	�޸ļ�¼��	V1.0��Э���װ�������ж϶���ͬһ���ļ������Ҳ�ͬЭ��ӿڲ�ͬ��
	*				V1.1���ṩͳһ�ӿڹ�Ӧ�ò�ʹ�ã����ݲ�ͬЭ���ļ�����װЭ����ص����ݡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
//#include "stm32f1xx_hal.h"
#include "main.h"

//�����豸
#include "esp8266.h"
 
//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

//Ӳ������
#include "usart.h"
//cJSON
#include "cjson.h"

//C��
#include <string.h>
#include <stdio.h>



#define PROID		"77249"    		//"77247"
//��Ʒ��	
#define AUTH_INFO	"test"

#define DEVID		"5616831"		//"5616839"
//�豸

extern unsigned char esp8266_buf[128];
char temp[70];
cJSON *json,*json_value;

extern uint8_t marks_LED,marks_BEEP,alarm_free;
uint8_t VAL_CMD=0;
char TAR_CMD[20];

//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

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
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ϴ�ƽ̨
		
		dataPtr = ESP8266_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:
						HAL_UART_Transmit(&huart1,(uint8_t *) "Tips:���ӳɹ�\r\n",strlen( "Tips:���ӳɹ�\r\n"), 1000);
						status = 0;
					break;
						//UsartPrintf(USART_DEBUG, "Tips:	���ӳɹ�\r\n")
					
					case 1:
						HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:����ʧ�ܣ�Э�����\r\n",strlen( "WARN:	����ʧ�ܣ�Э�����\r\n"), 1000);
					break;
					//UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�Э�����\r\n");
					case 2:HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n",strlen( "WARN:		����ʧ�ܣ��Ƿ���clientid\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");
					case 3:HAL_UART_Transmit(&huart1,(uint8_t *) "WARN:	����ʧ�ܣ�������ʧ��\r\n",strlen( "WARN:	����ʧ�ܣ� 	������ʧ��\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ�������ʧ��\r\n");
					case 4:HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:	����ʧ�ܣ��û������������\r\n",strlen("WARN:	����ʧ�ܣ��û������������\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��û������������\r\n");
					case 5:HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n",strlen("WARN:		����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");
					
					default:HAL_UART_Transmit(&huart1,(uint8_t *)"ERR:	����ʧ�ܣ�δ֪����\r\n",strlen("ERR:		����ʧ�ܣ�δ֪����\r\n"), 1000);break;
					//UsartPrintf(USART_DEBUG, "ERR:	����ʧ�ܣ�δ֪����\r\n");
				}
			}
		}
		else
		{
			memset(temp,0,70);
			sprintf(temp,"dataPtr == NULL\r\n");
			HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else	HAL_UART_Transmit(&huart1,(uint8_t *)"WARN:MQTT_PacketConnect Failed\r\n",strlen("WARN:MQTT_PacketConnect Failed\r\n"), 1000);
		//UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");

	return status;
	
}

//==========================================================
//	�������ƣ�	OneNet_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_SUBSCRIBE-��Ҫ�ط�
//
//	˵����		
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//Э���
	
	for(; i < topic_cnt; i++){
		memset(temp,0,70);
		sprintf(temp,"Subscribe Topic: %s\r\n", topics[i]);
		HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
	}
		//UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqttPacket);											//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNet_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_PUBLISH-��Ҫ����
//
//	˵����		
//==========================================================
void OneNet_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//Э���
	
	memset(temp,0,70);
	sprintf(temp,"Publish Topic: %s, Msg: %s\r\n", topic, msg);
	HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
	//UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqttPacket);											//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//Э���
	
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
		case MQTT_PKT_CMD:															//�����·�
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//���topic����Ϣ��
			if(result == 0)
			{
				memset(temp,0,70);
				sprintf(temp,"cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
				//UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//����ظ����
				{
					HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Send CmdResp\r\n",strlen("Tips:	Send CmdResp\r\n"), 1000);
					//UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ظ�����
					MQTT_DeleteBuffer(&mqttPacket);									//ɾ��
				}
			}
		
		break;
		
		case MQTT_PKT_PUBLISH:														//���յ�Publish��Ϣ
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				memset(temp,0,70);
				sprintf(temp,"topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				HAL_UART_Transmit(&huart1,(uint8_t *)temp,strlen(temp), 1000);
				//UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
				//												cmdid_topic, topic_len, req_payload, req_len);
				//�����ݰ�req_payload����JSON��ʽ����
				json = cJSON_Parse(req_payload);
				if(!json)//�����ݷ��ʹ��󱨸�
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
				cJSON_Delete(json);//ɾ������Ȼ��ը
			}
		
		break;
//		case MQTT_PKT_PUBLISH:														//���յ�Publish��Ϣ
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
//					case 1:															//�յ�publish��qosΪ1���豸��Ҫ�ظ�Ack
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
//					case 2:															//�յ�publish��qosΪ2���豸�Ȼظ�Rec
//																					//ƽ̨�ظ�Rel���豸�ٻظ�Comp
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
			
		case MQTT_PKT_PUBACK:														//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Publish Send OK\r\n",strlen("Tips:	MQTT Publish Send OK\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
		break;
			
		case MQTT_PKT_PUBREC:														//����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ
		
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
			
		case MQTT_PKT_PUBREL:														//�յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp
			
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
		
		case MQTT_PKT_PUBCOMP:														//����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	Rev PublishComp\r\n",strlen("Tips:	Rev PublishComp\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//����Subscribe��Ϣ��Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Subscribe OK\r\n",strlen("Tips:	MQTT Subscribe OK\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				HAL_UART_Transmit(&huart1,(uint8_t *)"Tips:	MQTT Subscribe Err\r\n",strlen("Tips:	MQTT Subscribe Err\r\n"), 1000);
				//UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:														//����UnSubscribe��Ϣ��Ack
		
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
	
	ESP8266_Clear();									//��ջ���
	
	if(result == -1)
		return;
	
	dataPtr = strchr(req_payload, '}');					//����'}'

	if(dataPtr != NULL && result != -1)					//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
	}

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
