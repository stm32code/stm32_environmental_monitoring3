// �����豸
#include "usart3.h"

// Э���ļ�
#include "Net.h"
#include "mqttkit.h"
#include "delay.h"

// Ӳ������
#include "usart.h"
#include "led.h"

//  C��
#include <string.h>
#include <stdio.h>

extern unsigned char esp8266_buf[512];

U8 Connect_Net;

//==========================================================
//	�������ƣ�	DevLink
//==========================================================
_Bool DevLink(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	unsigned char *dataPtr;

	_Bool status = 1;

	//printf("PROID: %s	AUIF: %s	DEVID:%s\r\n", PROID, AUTH_INFO, DEVID);

	if (MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 60, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // �ϴ�ƽ̨

		dataPtr = ESP8266_GetIPD(250); // �ȴ�ƽ̨��Ӧ
		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0:
					//printf("Tips:	���ӳɹ�\r\n");
					status = 0;
					Data_init.Error_Time = 0;
					break;

				case 1:
					//printf("WARN:	����ʧ�ܣ�Э�����\r\n");
					break;
				case 2:
					//printf("WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");
					break;
				case 3:
					//printf("WARN:	����ʧ�ܣ�������ʧ��\r\n");
					break;
				case 4:
					//printf("WARN:	����ʧ�ܣ��û������������\r\n");
					break;
				case 5:
					//printf("WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");
					break;

				default:
					//printf("ERR:	����ʧ�ܣ�δ֪����\r\n");
					break;
				}
			}
		}
		Data_init.Error_Time++;
		MQTT_DeleteBuffer(&mqttPacket); // ɾ��
	}
	else
	{
		Data_init.Error_Time++;
		//printf("WARN:	MQTT_PacketConnect Failed\r\n");
	}
	if (Data_init.Error_Time > 2)
	{
		Sys_Restart(); // ��λ
	}
	return status;
}

extern char CITY_SET[30]; // ����
unsigned char MQTT_FillBuf(char *buf)
{
	char text[256];
	memset(text, 0, sizeof(text));

	strcpy(buf, "{");
	//��Ч����
	memset(text, 0, sizeof(text));
	sprintf(text, "\"waning\":\"%d\",",device_state_init.Waning); 
	strcat(buf, text);
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"tmep_in\":\"%.1f\",",Data_init.temperatuer); 
	strcat(buf, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "\"humi_in\":\"%.1f\",",Data_init.humiditr); 
	strcat(buf, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "\"tmep_out\":\"%.1f\",", Data_init.temperatuer_out); 
	strcat(buf, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "\"city\":\"%s\"",CITY_SET); 
	strcat(buf, text);
	memset(text, 0, sizeof(text));
	sprintf(text, "}");
	strcat(buf, text);

	return strlen(buf);
}


//==========================================================

//	�������ܣ�	�ϴ����ݵ�ƽ̨

//==========================================================
void SendMqtt(U8 Cmd)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���
	char buf[254];
	short body_len = 0;
	memset(buf, 0, sizeof(buf));
	switch (Cmd)
	{
	case 1:
		body_len = MQTT_FillBuf(buf); // ������
		break;
	default:
		break;
	}

	if (body_len)
	{
		// ���
		if (MQTT_PacketPublish(MQTT_PUBLISH_ID, P_TOPIC_NAME, buf, body_len, MQTT_QOS_LEVEL1, 0, 1, &mqttPacket) == 0)
		{
			ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
			MQTT_DeleteBuffer(&mqttPacket);						 // ɾ��
		}

	}
}
//==========================================================

//	�������ܣ�	ƽ̨�������ݼ��

//==========================================================
void RevPro(unsigned char *cmd)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short topic_len = 0;
	unsigned short req_len = 0;

	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;

	short result = 0;

	//---------------------------------------------����һ����ȡ������������---------------------------------------------
	type = MQTT_UnPacketRecv(cmd);
	switch (type)
	{
		//---------------------------------------------����������ú�������-------------------------------------------------
	case MQTT_PKT_CMD: // �����·�

		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len); // ���topic����Ϣ��
		if (result == 0)
		{
			//printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0) // ����ظ����
			{
				//printf("Tips:	Send CmdResp\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
				MQTT_DeleteBuffer(&mqttPacket);						 // ɾ��
			}
		}

		break;

	case MQTT_PKT_PUBLISH: // ���յ�Publish��Ϣ

		result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
		if (result == 0)
		{
//			printf("topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
//				   cmdid_topic, topic_len, req_payload, req_len);

			switch (qos)
			{
			case 1: // �յ�publish��qosΪ1���豸��Ҫ�ظ�Ack

				if (MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
				{
					//printf("Tips:	Send PublishAck\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
					MQTT_DeleteBuffer(&mqttPacket);
				}

				break;

			case 2: // �յ�publish��qosΪ2���豸�Ȼظ�Rec
					// ƽ̨�ظ�Rel���豸�ٻظ�Comp
				if (MQTT_PacketPublishRec(pkt_id, &mqttPacket) == 0)
				{
					//printf("Tips:	Send PublishRec\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
					MQTT_DeleteBuffer(&mqttPacket);
				}
				break;

			default:
				break;
			}
		}

		break;

	case MQTT_PKT_PUBACK: // ����Publish��Ϣ��ƽ̨�ظ���Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
			//printf("Tips:	MQTT Publish Send OK\r\n");
		Connect_Net = 60; // �����ɹ�
		break;

	case MQTT_PKT_PUBREC: // ����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ

		if (MQTT_UnPacketPublishRec(cmd) == 0)
		{
			//printf("Tips:	Rev PublishRec\r\n");
			if (MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				//printf("Tips:	Send PublishRel\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}

		break;

	case MQTT_PKT_PUBREL: // �յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp

		if (MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
		{
			//printf("Tips:	Rev PublishRel\r\n");
			if (MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				//printf("Tips:	Send PublishComp\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}

		break;

	case MQTT_PKT_PUBCOMP: // ����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp

		if (MQTT_UnPacketPublishComp(cmd) == 0)
		{
			//printf("Tips:	Rev PublishComp\r\n");
		}

		break;

	case MQTT_PKT_SUBACK: // ����Subscribe��Ϣ��Ack

//		if (MQTT_UnPacketSubscribe(cmd) == 0)
//			printf("Tips:	MQTT Subscribe OK\r\n");
//		else
//			printf("Tips:	MQTT Subscribe Err\r\n");

		break;

	case MQTT_PKT_UNSUBACK: // ����UnSubscribe��Ϣ��Ack

//		if (MQTT_UnPacketUnSubscribe(cmd) == 0)
//			printf("Tips:	MQTT UnSubscribe OK\r\n");
//		else
//			printf("Tips:	MQTT UnSubscribe Err\r\n");

		break;

	default:
		result = -1;
		break;
	}
	//---------------------------------------------�������������---------------------------------------------

	ESP8266_Clear(); // ��ջ���

	if (result == -1)
		return;
	// ����json
	if (req_payload != NULL)
	{
		massage_parse_json((char *)req_payload);
	}
	//---------------------------------------------�����ģ��ͷ��ڴ�---------------------------------------------
	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}

//==========================================================
//	�������ܣ�	�����ж�
//==========================================================
void Link_OneNet(u8 Link)
{
	if (!Link)
	{
		ESP8266_Init();			 // ��ʼ��ESP8266
		while (DevLink()) // ����OneNET
			delay_ms(500);
	}
}
//==========================================================
//	�������ƣ�	Subscribe
//==========================================================
_Bool Subscribe(const char *topics[], unsigned char topic_cnt)
{

	unsigned char i = 0;
	unsigned char *dataPtr;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

//	for (; i < topic_cnt; i++)
//		printf("Subscribe Topic: %s\r\n", topics[i]);

	//---------------------------------------------����һ�����---------------------------------------------
	if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		//---------------------------------------------���������������-----------------------------------------
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
		dataPtr = ESP8266_GetIPD(500);						 // �ȴ�ƽ̨��Ӧ
		if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_SUBACK)
		{
			//---------------------------------------------��������ɾ��---------------------------------------------
			//printf("Tips:	MQTT Subscribe OK\r\n");
			MQTT_DeleteBuffer(&mqttPacket); // ɾ��
			Data_init.Error_Time = 0;
			return 0;
		}
		else
		{
			//---------------------------------------------��������ɾ��---------------------------------------------
			//printf("Tips:	MQTT Subscribe Err\r\n");
			MQTT_DeleteBuffer(&mqttPacket); // ɾ��
			if (++Data_init.Error_Time > 2)
			{
				Sys_Restart(); // ��λ
			}
			return 1;
		}
	}

	return 1;
}

//==========================================================
//	�������ܣ�	������Ϣ
//==========================================================
_Bool Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	// printf( "Publish Topic: %s, Msg: %s\r\n", topic, msg);

	if (MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL1, 0, 1, &mqttPacket) != 1)
	{

		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������
		MQTT_DeleteBuffer(&mqttPacket);						 // ɾ��
	}
	return 0;
}
