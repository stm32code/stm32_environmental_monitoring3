#include "git.h"


// �����ʱ���趨
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;
extern u8 time25ms;

// ��ȡȫ�ֱ���
const char *topics[] = {S_TOPIC_NAME};

// Ӳ����ʼ��
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    HZ = GB16_NUM();                                // ����
    delay_init();                                   // ��ʱ������ʼ��
    GENERAL_TIM_Init(TIM_4, 0, 1);
    Usart1_Init(9600 ); // ����1��ʼ��Ϊ115200
		Usart2_Init(9600,1,2);   // ����
    Usart3_Init(115200); // ����3������ESP8266��
    System_PB34_setIO();
    LED_Init();
    Key_GPIO_Config(); // key
		DHT11_GPIO_Config(); //��ʪ��
		DS1302_Init();      // ��ʼ��ds1302�˿�
	  Level_GPIO_Config();//���ֺ�����
	
		BEEP = 0;
#if OLED // OLED�ļ�����
    OLED_Init();
    OLED_ColorTurn(0);   // 0������ʾ��1 ��ɫ��ʾ
    OLED_DisplayTurn(0); // 0������ʾ 1 ��Ļ��ת��ʾ
#endif
    while (Reset_Threshole_Value(&threshold_value_init, &device_state_init) != MY_SUCCESSFUL)
        delay_ms(5); // ��ʼ����ֵ
  	printf("��ӭʹ�� ");
#if OLED // OLED�ļ�����
    OLED_Clear();
#endif
		

}
// �����ʼ��
void Net_Init()
{
		 char str[50];
// �豸����
#if NETWORK_CHAEK
    if (Connect_Net == 0) {

       OLED_Clear();
			// дOLED����
			sprintf(str, "-����:%s         ", SSID);
			OLED_ShowCH(0, 2, (unsigned char *)str);
			sprintf(str, "-����:%s         ", PASS);
			OLED_ShowCH(0, 4, (unsigned char *)str);
			sprintf(str, "-Ƶ��: 2.4 GHz   ");
			OLED_ShowCH(0, 6, (unsigned char *)str);
			
			// ��ȡ����
			sprintf(str, "   ������ȡ��  " );  // Data_init.temperatuer
			OLED_ShowCH(0, 0, (unsigned char *)str);
			GetWeather();
			Weather();
			
			sprintf(str, "   ƽ̨������  " );  // Data_init.temperatuer
			OLED_ShowCH(0, 0, (unsigned char *)str);
			ESP8266_Init();          // ��ʼ��ESP8266
			while (DevLink()) // ����ƽ̨
					delay_ms(300);
			while (Subscribe(topics, 1)) // ��������
					delay_ms(300);

			Connect_Net = 60; // �����ɹ�
			OLED_Clear();

    }
#endif

}

// ����1
void task1(void)
{

		Net_Init();		// ����
    Read_Data(&Data_init);   // ���´���������
    Update_oled_massage();   // ����OLED
    Update_device_massage(); // �����豸
                             // BEEP= ~BEEP;
    State = ~State;
	
}
// ����2
void task2(void)
{
	//1�������
 	Automation_Close();
	
}
// ����3
void task3(void)
{
  // �����ϴ�����Ϣʱ�� ��ֹ�ظ�����
	Data_init.SendTime++; //�ṩʱ��
	if(Data_init.SendTime > 3){
		Data_init.AppTime ++;
		if(Data_init.AppTime > 1 ){
			Data_init.AppTime = 1; // ��Բ�ͬ���Ƶ�����
		}
		// ������Ϣ
		if (Connect_Net && Data_init.App == 0) {
				 Data_init.App = Data_init.AppTime;
		}
	}
}
// �����ʼ��
void SoftWare_Init(void)
{
    // ��ʱ����ʼ��
    timer_init(&task1_id, task1, 200, 1); // 200msִ��һ��
    timer_init(&task2_id, task2, 1000, 1);  // 1sִ��һ��
    timer_init(&task3_id, task3, 1500, 1); // 1.5sִ��һ��

    timer_start(&task1_id);
    timer_start(&task2_id);
    timer_start(&task3_id);
}
// ������
int main(void)
{

    unsigned char *dataPtr = NULL;
    SoftWare_Init(); // �����ʼ��
    Hardware_Init(); // Ӳ����ʼ��
    // ������ʾ
    Net_Init(); // �����ʼ
    TIM_Cmd(TIM4, ENABLE); // ʹ�ܼ�����
    while (1) {

        // �߳�
        timer_loop(); // ��ʱ��ִ��
        // ���ڽ����ж�
        dataPtr = ESP8266_GetIPD(0);
        if (dataPtr != NULL) {
            RevPro(dataPtr); // ��������
        }
				// ��������
				Send_Usart2();
#if KEY_OPEN
				// �������
				if(time25ms == MY_TRUE){
						Check_Key_ON_OFF();
						time25ms = MY_FALSE;
				}
#endif
    }
}

