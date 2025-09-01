#include "git.h"


// 软件定时器设定
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;
extern u8 time25ms;

// 获取全局变量
const char *topics[] = {S_TOPIC_NAME};

// 硬件初始化
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    HZ = GB16_NUM();                                // 字数
    delay_init();                                   // 延时函数初始化
    GENERAL_TIM_Init(TIM_4, 0, 1);
    Usart1_Init(9600 ); // 串口1初始化为115200
		Usart2_Init(9600,1,2);   // 语音
    Usart3_Init(115200); // 串口3，驱动ESP8266用
    System_PB34_setIO();
    LED_Init();
    Key_GPIO_Config(); // key
		DHT11_GPIO_Config(); //温湿度
		DS1302_Init();      // 初始化ds1302端口
	  Level_GPIO_Config();//火灾和烟雾
	
		BEEP = 0;
#if OLED // OLED文件存在
    OLED_Init();
    OLED_ColorTurn(0);   // 0正常显示，1 反色显示
    OLED_DisplayTurn(0); // 0正常显示 1 屏幕翻转显示
#endif
    while (Reset_Threshole_Value(&threshold_value_init, &device_state_init) != MY_SUCCESSFUL)
        delay_ms(5); // 初始化阈值
  	printf("欢迎使用 ");
#if OLED // OLED文件存在
    OLED_Clear();
#endif
		

}
// 网络初始化
void Net_Init()
{
		 char str[50];
// 设备重连
#if NETWORK_CHAEK
    if (Connect_Net == 0) {

       OLED_Clear();
			// 写OLED内容
			sprintf(str, "-名称:%s         ", SSID);
			OLED_ShowCH(0, 2, (unsigned char *)str);
			sprintf(str, "-密码:%s         ", PASS);
			OLED_ShowCH(0, 4, (unsigned char *)str);
			sprintf(str, "-频率: 2.4 GHz   ");
			OLED_ShowCH(0, 6, (unsigned char *)str);
			
			// 获取天气
			sprintf(str, "   天气获取中  " );  // Data_init.temperatuer
			OLED_ShowCH(0, 0, (unsigned char *)str);
			GetWeather();
			Weather();
			
			sprintf(str, "   平台连接中  " );  // Data_init.temperatuer
			OLED_ShowCH(0, 0, (unsigned char *)str);
			ESP8266_Init();          // 初始化ESP8266
			while (DevLink()) // 接入平台
					delay_ms(300);
			while (Subscribe(topics, 1)) // 订阅主题
					delay_ms(300);

			Connect_Net = 60; // 入网成功
			OLED_Clear();

    }
#endif

}

// 任务1
void task1(void)
{

		Net_Init();		// 网络
    Read_Data(&Data_init);   // 更新传感器数据
    Update_oled_massage();   // 更新OLED
    Update_device_massage(); // 更新设备
                             // BEEP= ~BEEP;
    State = ~State;
	
}
// 任务2
void task2(void)
{
	//1秒计算器
 	Automation_Close();
	
}
// 任务3
void task3(void)
{
  // 计算上传发消息时间 防止重复触发
	Data_init.SendTime++; //提供时间
	if(Data_init.SendTime > 3){
		Data_init.AppTime ++;
		if(Data_init.AppTime > 1 ){
			Data_init.AppTime = 1; // 针对不同类似的数据
		}
		// 发送消息
		if (Connect_Net && Data_init.App == 0) {
				 Data_init.App = Data_init.AppTime;
		}
	}
}
// 软件初始化
void SoftWare_Init(void)
{
    // 定时器初始化
    timer_init(&task1_id, task1, 200, 1); // 200ms执行一次
    timer_init(&task2_id, task2, 1000, 1);  // 1s执行一次
    timer_init(&task3_id, task3, 1500, 1); // 1.5s执行一次

    timer_start(&task1_id);
    timer_start(&task2_id);
    timer_start(&task3_id);
}
// 主函数
int main(void)
{

    unsigned char *dataPtr = NULL;
    SoftWare_Init(); // 软件初始化
    Hardware_Init(); // 硬件初始化
    // 启动提示
    Net_Init(); // 网络初始
    TIM_Cmd(TIM4, ENABLE); // 使能计数器
    while (1) {

        // 线程
        timer_loop(); // 定时器执行
        // 串口接收判断
        dataPtr = ESP8266_GetIPD(0);
        if (dataPtr != NULL) {
            RevPro(dataPtr); // 接收命令
        }
				// 语音控制
				Send_Usart2();
#if KEY_OPEN
				// 按键监测
				if(time25ms == MY_TRUE){
						Check_Key_ON_OFF();
						time25ms = MY_FALSE;
				}
#endif
    }
}

