#ifndef  __API_H
#define	 __API_H

#include "stm32f10x.h"

/********************************** �û���Ҫ���õĲ���**********************************/

//����ʱ����������˿�
//#define      BEIJING_TIME_IP            "103.205.4.43"      //Ҫ���ӵķ������� IP
#define      BEIJING_TIME_IP            "api.nowapi.com"      //Ҫ���ӵķ������� IP
#define      BEIJING_TIME_Port         	 80               //Ҫ���ӵķ������Ķ˿�

//�������������˿�
//#define      GUANGZHOU_WEATHER_IP                 "116.62.81.138"      //Ҫ���ӵķ������� IP
#define      GUANGZHOU_WEATHER_IP                 "api.seniverse.com"      //Ҫ���ӵķ������� IP
#define      GUANGZHOU_WEATHER_Port            	   80               //Ҫ���ӵķ������Ķ˿�

//==========================================================
//	�������ƣ�	֪�ķ���������
//==========================================================
void Weather_Init(void);
//֪����������api�ӿ�
void GetWeather(void);
//֪������δ��api�ӿ�
void Weather(void);
#endif
