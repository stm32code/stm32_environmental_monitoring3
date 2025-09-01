#ifndef  __API_H
#define	 __API_H

#include "stm32f10x.h"

/********************************** 用户需要设置的参数**********************************/

//北京时间服务器及端口
//#define      BEIJING_TIME_IP            "103.205.4.43"      //要连接的服务器的 IP
#define      BEIJING_TIME_IP            "api.nowapi.com"      //要连接的服务器的 IP
#define      BEIJING_TIME_Port         	 80               //要连接的服务器的端口

//天气服务器及端口
//#define      GUANGZHOU_WEATHER_IP                 "116.62.81.138"      //要连接的服务器的 IP
#define      GUANGZHOU_WEATHER_IP                 "api.seniverse.com"      //要连接的服务器的 IP
#define      GUANGZHOU_WEATHER_Port            	   80               //要连接的服务器的端口

//==========================================================
//	函数名称：	知心服务器连接
//==========================================================
void Weather_Init(void);
//知心天气当日api接口
void GetWeather(void);
//知心天气未来api接口
void Weather(void);
#endif
