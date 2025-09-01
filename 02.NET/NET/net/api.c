#include "api.h"

#include "oled.h"
// �����豸����
#include "usart3.h"
// C��
#include "stdio.h"  
#include "string.h"  
#include "stdbool.h"
#include "stdlib.h"


char cStr [ 512 ] = { 0 };
extern unsigned char esp8266_buf[512]; // ��������
char *p1=NULL;
char *p2=NULL;
char weather=99; // �����ش�
char weather_flag=1;
extern char CITY_SET[30]; // ��ȡ��λ�ĳ���

//==========================================================
//	�������ƣ�	֪�ķ���������
//==========================================================
void Weather_Init(void)
{
	char str[125];

	delay_ms(250);

	ESP8266_Clear();

	//printf("AT\r\n");
	while (ESP8266_SendCmd("AT\r\n\r", "OK", 200))
		delay_ms(300);

	//printf("CWMODE\r\n");
	while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))
		delay_ms(300);

	//printf("AT+CWDHCP\r\n");
	while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))
		delay_ms(300);

	//printf("CWJAP\r\n");
	memset(str, 0, sizeof(str));
	sprintf(str, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASS);
	while (ESP8266_SendCmd(str, "GOT IP", 200))
		delay_ms(300);

	//printf("CIPSTART\r\n");
	memset(str, 0, sizeof(str));
	sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", GUANGZHOU_WEATHER_IP, GUANGZHOU_WEATHER_Port);
	while (ESP8266_SendCmd(str, "CONNECT", 200))
		delay_ms(500);
	//printf("Weather Init OK\r\n");

}
// ���ݸ�ʽ��
char c2d(uint8_t c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}
uint16_t atoi16(char* str,uint16_t base )
{
  unsigned int num = 0;
  while (*str !=0)
          num = num * base + c2d(*str++);
  return num;
}
/*
{
    "results": [
        {
            "location": {
                "id": "WM6N2PM3WY2K",
                "name": "�ɶ�",
                "country": "CN",
                "path": "�ɶ�,�ɶ�,�Ĵ�,�й�",
                "timezone": "Asia/Shanghai",
                "timezone_offset": "+08:00"
            },
            "daily": [
                {
                    "date": "2024-11-12",
                    "text_day": "����",
                    "code_day": "4",
                    "text_night": "��",
                    "code_night": "9",
                    "high": "19",
                    "low": "15",
                    "rainfall": "0.00",
                    "precip": "0.09",
                    "wind_direction": "��",
                    "wind_direction_degree": "0",
                    "wind_speed": "8.4",
                    "wind_scale": "2",
                    "humidity": "96"
                },
                {
                    "date": "2024-11-13",
                    "text_day": "����",
                    "code_day": "4",
                    "text_night": "��",
                    "code_night": "1",
                    "high": "20",
                    "low": "13",
                    "rainfall": "0.00",
                    "precip": "0.10",
                    "wind_direction": "��",
                    "wind_direction_degree": "0",
                    "wind_speed": "8.4",
                    "wind_scale": "2",
                    "humidity": "98"
                },
                {
                    "date": "2024-11-14",
                    "text_day": "����",
                    "code_day": "4",
                    "text_night": "��",
                    "code_night": "9",
                    "high": "22",
                    "low": "13",
                    "rainfall": "0.00",
                    "precip": "0.10",
                    "wind_direction": "��",
                    "wind_direction_degree": "0",
                    "wind_speed": "3.0",
                    "wind_scale": "1",
                    "humidity": "100"
                }
            ],
            "last_update": "2024-11-12T08:00:00+08:00"
        }
    ]
}
*/
// �������� ����
char code_days[3][10]; // ���ڴ洢����code_day��ֵ
// ��ȡcode_day�ĺ���
void extract_code_days(const char *json_data) {
    int index = 0;
    const char *start = json_data;
    const char *end;
    while (index < 3 && (start = strstr(start, "\"code_day\":\"")) != NULL) {
        start += strlen("\"code_day\":\"");
        end = strchr(start, '"');
        if (end != NULL && (end - start) < sizeof(code_days[index])) {
            strncpy(code_days[index], start, end - start);
            code_days[index][end - start] = '\0'; // ȷ���ַ�����null��β
            index++;
        } else {
            break; // ����Ҳ������ʵĽ������Ż��ַ���̫������ֹͣ����
        }
        start = end + 1; // ����������һ��code_day
    }
    // ��ӡ��ȡ��code_dayֵ
    for (int i = 0; i < index; i++) {
        //printf("code_day[%d]: %s\n", i, code_days[i]);
    }
}

// ��������ɾ���ַ����е�һ�γ��ֵ�ָ�����ȵ����ַ���
void removeSubstrOnce(char *str, const char *substr) {
    char temp[1200] = {0}; // ��ʱ�ַ���������Դ�ַ������ᳬ��1200���ַ�
    char *pos = strstr(str, substr); // �������ַ�����λ��
    if (pos != NULL) {
        strncpy(temp, str, pos - str); // �������ַ���֮ǰ������
        temp[pos - str] = '\0'; // ����ַ�����ֹ��
        strcat(temp, pos + strlen(substr)); // �������ַ���֮�������
        strcpy(str, temp); // ���޸ĺ���ַ������ƻ�ԭ�ַ���
    }
}
// δ������
// api.seniverse.com/v3/weather/daily.json?key=SNwcvvzklSwlu3JNS&location=%s&language=zh-Hans&unit=c&start=0&days=5 chengdu
void Weather(void)
{
  unsigned char *dataPtr = NULL;
	Weather_Init(); // ���������

	weather_flag=1;

	while(weather_flag)
	{
		sprintf ( cStr,"GET https://api.seniverse.com/v3/weather/daily.json?key=Sl9gXqsTN_31RZUIr&location=%s&language=zh-Hans&unit=c&start=0&days=5\n",CITY_SET); // CITY_SET
		ESP8266_SendData ((unsigned char*)cStr, strlen(cStr) );               //��������
		dataPtr = API_GetIPD(100);
		delay_ms ( 500 );
		// ɾ�� CLOSE
		removeSubstrOnce((char * )dataPtr,"CLOSED");
		//printf ( "%s\r\n", dataPtr );
		if (strstr((char*)dataPtr,"last_update")) {
				weather_flag=0;
		}
	}
	extract_code_days((char*)dataPtr);
	
	ESP8266_Clear(); // ��ջ���
	ESP8266_SendCmd ( "AT+CIPCLOSE", "OK", 50 ); // �ر�����
	
}
//֪������api�ӿ�
void GetWeather(void)
{


	Weather_Init(); // ���������

	weather_flag=1;

	while(weather_flag)
	{
		sprintf ( cStr,"GET https://api.seniverse.com/v3/weather/now.json?key=Sl9gXqsTN_31RZUIr&location=%s&language=zh-Hans&unit=c\n",CITY_SET); // CITY_SET
		ESP8266_SendData ((unsigned char*)cStr, strlen(cStr) );               //��������
		delay_ms ( 200 );
		//printf ( "%s\r\n", esp8266_buf );
		if(strstr((char*)esp8266_buf,"last_update"))
		{
			weather_flag=0;
		}
	}
	p2=strtok((char *)esp8266_buf,"_");
	
	p2=strtok(NULL,",");
	
	p2=strtok(NULL,",");
	
	p2=strtok(NULL,"d");	
	p2=strtok(NULL,"\"");
	
	p2=strtok(NULL,"\"");
	p2=strtok(NULL,"\"");
	
	//printf ( "\r\n%s\r\n", p2 );

	weather=atoi16(p2,10);
	
	p2=strtok(NULL,"r");	
	p2=strtok(NULL,"\"");
	
	p2=strtok(NULL,"\"");
	p2=strtok(NULL,"\"");
	
	Data_init.temperatuer_out=atoi16(p2,10); // �����¶�
	
	Data_init.Weather = weather; // ����״̬

	
	ESP8266_Clear(); // ��ջ���
	ESP8266_SendCmd ( "AT+CIPCLOSE", "OK", 50 ); // �ر�����
	
}

