#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "rtc.h"
#include "beep.h"
#include "stmflash.h"
#include "dht11.h"
#include "timer3.h"
#include "timer4.h"
#include "step.h"
#include "wdg.h"

#define FLASH_SAVE_ADDR 0X08070000 //设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

// 配置的结构体
typedef struct Config
{
	u16 temp_max;
	u16 rh_max;
	u16 rh_min;
	u16 water_time;
	u16 rest_time;
	u16 alarm_time;
}Config;

/* 显示时间，index特殊处理 */
void show_msg(u8 index, _calendar_obj calendar_temp, u16 temp, u16 rh, Config config);

int main(void)
{
    /* 按键返回值 */
    u8 key = 0;
    /* 修改指向下标 */
    u8 index = 0;
    /* 日历结构体 */
    _calendar_obj calendar_temp;
    // 修改模式标志
    u8 mode = 0;
	// 计时用秒
	u16 sec = 0;
	// 工作标志位
	u8 work_flag = 0;
	// 休息标志位
	u8 rest_flag = 0;
	u16 temp = 0;
	u16 rh = 0;
	Config config = {40, 80, 50, 1, 1, 1};
	// 电机标志位 1为正转了90度，浇水状态 0为关水状态
	u8 motor_flag = 0;

    /* 延时函数初始化 */
    delay_init();
    /* 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 串口初始化为115200 */
    uart_init(115200);
    /* LED端口初始化 */
    LED_Init();
    /* LCD初始化 */
    LCD_Init();
    /* 按键初始化 */
    KEY_Init();
    /* RTC初始化 */
    RTC_Init();
    /* 初始化蜂鸣器端口 */
    BEEP_Init();
	// 10Khz的计数频率，计数到500为50ms  
	TIM3_Int_Init(499, 7199);
	// 独立看门狗初始化 预分频数为4*2^1=8,重载值为625,溢出时间为125ms
	IWDG_Init(1,625);    
	// 10Khz 的计数频率，计数到 1000 为 100ms
	TIM4_Int_Init(999, 7199); 
	/*                             
		步进电机初始化
		IN4: PF4  d
		IN3: PF3  c
		IN2: PF2  b
		IN1: PF1  a
	*/
	Step_Motor_GPIO_Init();
	
	LED0 = 1;
	
	// DHT11初始化 DATA -> PG11
	while(DHT11_Init())	
    {
        LCD_ShowString(30,130,200,16,16,"DHT11 Error");
        delay_ms(200);
        LCD_Fill(30,130,239,130+16,WHITE);
        delay_ms(200);
    }
	
	DHT11_Read_Data((u8 *)&temp, (u8 *)&rh);

    /*
    * 显示时间
    * 设置字体为蓝色
    */
    POINT_COLOR = BLUE;
    LCD_ShowString(60, 40, 300, 16, 16, "NOW      :  :  ");
	LCD_ShowString(60, 66, 100, 16, 16, "Temp:     CEL");
	LCD_ShowString(60, 82, 100, 16, 16, "RH  :     %RH");
    LCD_ShowString(60, 162, 200, 12, 12, "Temp   Max:    CEL");
	LCD_ShowString(60, 174, 200, 12, 12, "RH     Max:    %RH");
	LCD_ShowString(60, 186, 200, 12, 12, "RH     Min:    %RH");
	LCD_ShowString(60, 198, 200, 12, 12, "Water Time:    min");
	LCD_ShowString(60, 210, 200, 12, 12, "Rest  Time:    min");
	LCD_ShowString(60, 222, 200, 12, 12, "Alarm Time:    sec");
	POINT_COLOR = BLACK;
	LCD_ShowString(60, 260, 200, 16, 16, "Mode: ");
	POINT_COLOR = BLUE;
    calendar_temp = calendar;	
	
	delay_ms(1000);
	DHT11_Read_Data((u8 *)&temp, (u8 *)&rh);
	delay_ms(1000);
	DHT11_Read_Data((u8 *)&temp, (u8 *)&rh);

    // 从指定地址开始读出指定长度的数据
    // ReadAddr:起始地址
    // pBuffer:数据指针
    // NumToWrite:半字(16位)数
    STMFLASH_Read(FLASH_SAVE_ADDR, &config.temp_max, 1);
    STMFLASH_Read(FLASH_SAVE_ADDR + 0X2, &config.rh_max, 1);
	STMFLASH_Read(FLASH_SAVE_ADDR + 0X4, &config.rh_min, 1);
	STMFLASH_Read(FLASH_SAVE_ADDR + 0X6, &config.water_time, 1);
	STMFLASH_Read(FLASH_SAVE_ADDR + 0X8, &config.rest_time, 1);
	STMFLASH_Read(FLASH_SAVE_ADDR + 0X10, &config.alarm_time, 1);
	
	// flash初始值
	if(65535 == config.temp_max && 65535 == config.rh_max && 65535 == config.rh_min && 65535 == config.water_time && 65535 == config.rest_time && 65535 == config.alarm_time)
	{
		config.temp_max = 40;
		config.rh_max = 80;
		config.rh_min = 50;
		config.water_time = 1;
		config.rest_time = 1;
		config.alarm_time = 1;
	}
	
	printf("config.temp_max = %d, config.rh_max = %d, config.rh_min = %d, config.water_time = %d, config.rest_time = %d, config.alarm_time = %d\r\n",
		config.temp_max, config.rh_max, config.rh_min, config.water_time, config.rest_time, config.alarm_time);

    while (1)
    {
		if(work_flag == 1)
		{
			POINT_COLOR = RED;
			LCD_ShowString(108, 260, 100, 16, 16, "Working");
		}
		else if(rest_flag == 1)
		{
			POINT_COLOR = GREEN;
			LCD_ShowString(108, 260, 100, 16, 16, "Resting");
		}
		else
		{
			POINT_COLOR = BLACK;
			LCD_ShowString(108, 260, 100, 16, 16, "Running");
		}
		POINT_COLOR = BLUE;
		
		if(time % 40 == 0)
		{
			// 读取温湿度值
			DHT11_Read_Data((u8 *)&temp, (u8 *)&rh);
			// printf("Temp:%dCEL , Humidity:%d%%RH\r\n", temp, rh);
			
			LED0 = !LED0;
		}
		
		// 约1秒 工作或休息计时中时
		if(time % 20 == 0 && (work_flag == 1 || rest_flag == 1))
		{
			sec++;
		}
		
		// 不在工作和休息中，清空计时
		if(work_flag == 0 && rest_flag == 0)
		{
			sec = 0;
		}
		
		// 工作时间到达
		if(work_flag == 1)
		{
			if(sec >= config.alarm_time)
			{
				BEEP = 0;
			}
			
			if(sec / 60 >= config.water_time)
			{
				// 停止浇水
				LED1 = 1;
				printf("****  water finish  ****\r\n");
				// 工作标志位置0
				work_flag = 0;
				// 休息标志位置1
				rest_flag = 1;
				
				if(motor_flag == 1)
				{
					// 反转90度
					motor_circle(16, 0, 2);
					motor_flag = 0;
				}
			}
		}
		
		// 工作完后 休息时间到达
		if(rest_flag == 1 && sec / 60 >= (config.water_time + config.rest_time))
		{
			printf("****  rest finish  ****\r\n");
			sec = 0;
			rest_flag = 0;
		}
		
        /* 根据index显示 */
        if (0 == index)
            show_msg(index, calendar, temp, rh, config);
        else
            show_msg(index, calendar_temp, temp, rh, config);

        /*
        * 键处理函数
        * 返回按键值
        * mode:0,不支持连续按;1,支持连续按;
        * 0，没有任何按键按下
        * 1，KEY0按下
        * 2，KEY1按下
        * 3，KEY3按下 WK_UP
        */
        key = KEY_Scan(1);
        /* KEY0 进入修改模式，依次顺序循环 */
        if (1 == key)
        {
            index++;
            index = index % 10;
            /* 进入修改 */
            if (1 == index)
            {
                calendar_temp = calendar;
                mode = 1;
            }
            /* 退出修改 */
            else if (0 == index)
            {
                calendar = calendar_temp;
                RTC_Set(calendar_temp.w_year, calendar_temp.w_month, calendar_temp.w_date, calendar_temp.hour, calendar_temp.min, calendar_temp.sec);
                // 从指定地址开始写入指定长度的数据
                // WriteAddr:起始地址(此地址必须为2的倍数!!)
                // pBuffer:数据指针
                // NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
				STMFLASH_Write(FLASH_SAVE_ADDR, &config.temp_max, 1);
				STMFLASH_Write(FLASH_SAVE_ADDR + 0X2, &config.rh_max, 1);
				STMFLASH_Write(FLASH_SAVE_ADDR + 0X4, &config.rh_min, 1);
				STMFLASH_Write(FLASH_SAVE_ADDR + 0X6, &config.water_time, 1);
				STMFLASH_Write(FLASH_SAVE_ADDR + 0X8, &config.rest_time, 1);
				STMFLASH_Write(FLASH_SAVE_ADDR + 0X10, &config.alarm_time, 1);
                mode = 0;
            }
        }
        /* KEY1 选中值+1 */
        else if (2 == key)
        {
            if (1 == index)
            {
                calendar_temp.hour++;
                calendar_temp.hour = calendar_temp.hour > 23 ? 0 : calendar_temp.hour;
            }
            else if (2 == index)
            {
                calendar_temp.min++;
                calendar_temp.min = calendar_temp.min > 59 ? 0 : calendar_temp.min;
            }
            else if (3 == index)
            {
                calendar_temp.sec++;
                calendar_temp.sec = calendar_temp.sec > 59 ? 0 : calendar_temp.sec;
            }
            else if (4 == index)
            {
                config.temp_max++;
                config.temp_max = config.temp_max > 99 ? 0 : config.temp_max;
            }
            else if (5 == index)
            {
                config.rh_max++;
                config.rh_max = config.rh_max > 99 ? (config.rh_min+1) : config.rh_max;
            }
			else if (6 == index)
            {
                config.rh_min++;
                config.rh_min = config.rh_min > (config.rh_max-1) ? 0 : config.rh_min;
            }
			else if (7 == index)
            {
                config.water_time++;
                config.water_time = config.water_time > 9 ? 1 : config.water_time;
            }
			else if (8 == index)
            {
                config.rest_time++;
                config.rest_time = config.rest_time > 9 ? 1 : config.rest_time;
            }
			else if (9 == index)
            {
                config.alarm_time++;
                config.alarm_time = config.alarm_time > 9 ? 1 : config.alarm_time;
            }
			// 普通模式下
            else if (0 == index)
            {
				printf("****  KEY1 press, start rest  ****\r\n");
				LED1 = 1;
				work_flag = 0;
				// 休息计时开启
				rest_flag = 1;
				
				if(motor_flag == 1)
				{
					// 反转90度
					motor_circle(16, 0, 2);
					motor_flag = 0;
				}
            }
        }
        /* KEY_UP 选中值-1 */
        else if (3 == key)
        {
            if (1 == index)
            {
                calendar_temp.hour = calendar_temp.hour == 0 ? 23 : calendar_temp.hour - 1;
            }
            else if (2 == index)
            {
                calendar_temp.min = calendar_temp.min == 0 ? 59 : calendar_temp.min - 1;
            }
            else if (3 == index)
            {
                calendar_temp.sec = calendar_temp.sec == 0 ? 59 : calendar_temp.sec - 1;
            }
            else if (4 == index)
            {
                config.temp_max = config.temp_max == 0 ? 100 : config.temp_max - 1;
            }
            else if (5 == index)
            {
                config.rh_max = config.rh_max == config.rh_min ? 100 : config.rh_max - 1;
            }
			else if (6 == index)
            {
                config.rh_min = config.rh_min == 0 ? config.rh_max - 1 : config.rh_min - 1;
            }
			else if (7 == index)
            {
                config.water_time = config.water_time == 0 ? 1 : config.water_time - 1;
            }
			else if (8 == index)
            {
                config.rest_time = config.rest_time == 0 ? 1 : config.rest_time - 1;
            }
			else if (9 == index)
            {
                config.alarm_time = config.alarm_time == 0 ? 1 : config.alarm_time - 1;
            }
            else if (0 == index)
            {
                printf("****  KEY_UP press, start work  ****\r\n");
				LED1 = 0;
				BEEP = 1;
				work_flag = 1;
				
				if(motor_flag == 0)
				{
					// 正转90度
					motor_circle(16, 1, 2);
					motor_flag = 1;
				}
            }
        }
        else if (0 == key)
        {
            // 普通模式下
            if (0 == mode)
            {
				// 湿度过低 且 没有在工作计时和休息计时中
				if(rh <= config.rh_min && work_flag == 0 && rest_flag == 0)
				{
					printf("****  The humidity is too low, start watering  ****\r\n");
					BEEP = 1;
					work_flag = 1;
					
					// 模拟浇水
					LED1 = 0;
					if(motor_flag == 0)
					{
						// 正转90度
						motor_circle(16, 1, 2);
						motor_flag = 1;
					}
				}
				// 湿度合适但温度过高 且 没有在工作计时和休息计时中
				else if(config.rh_max > rh && rh > config.rh_min && temp >= config.temp_max && work_flag == 0 && rest_flag == 0)
				{
					printf("****  The temperature is too high, start watering  ****\r\n");
					BEEP = 1;
					work_flag = 1;
					
					// 模拟浇水
					LED1 = 0;
					if(motor_flag == 0)
					{
						// 正转90度
						motor_circle(16, 1, 2);
						motor_flag = 1;
					}
				}
				
				// 湿度过高 且 不在休息计时中
				if(rh >= config.rh_max && rest_flag == 0)
				{
					printf("****  If the humidity is too high, stop watering  ****\r\n");
					work_flag = 0;
					// 休息计时开启
					rest_flag = 1;
					
					// 停止浇水
					LED1 = 1;
					if(motor_flag == 1)
					{
						// 反转90度
						motor_circle(16, 0, 2);
						motor_flag = 0;
					}
				}
            }
        }

        delay_ms(20);
    }
}

/* 显示信息，index特殊处理 */
void show_msg(u8 index, _calendar_obj calendar_temp, u16 temp, u16 rh, Config config)
{
    POINT_COLOR = BLUE;
    if (1 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(116, 40, calendar_temp.hour, 2, 16);
    POINT_COLOR = BLUE;
    if (2 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(140, 40, calendar_temp.min, 2, 16);
    POINT_COLOR = BLUE;
    if (3 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(164, 40, calendar_temp.sec, 2, 16);
    POINT_COLOR = BLUE;
	if (4 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 162, config.temp_max, 2, 12);
    POINT_COLOR = BLUE;
	if (5 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 174, config.rh_max, 2, 12);
    POINT_COLOR = BLUE;
	if (6 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 186, config.rh_min, 2, 12);
    POINT_COLOR = BLUE;
	if (7 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 198, config.water_time, 2, 12);
    POINT_COLOR = BLUE;
	if (8 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 210, config.rest_time, 2, 12);
    POINT_COLOR = BLUE;
	if (9 == index)
        POINT_COLOR = RED;
    LCD_ShowNum(132, 222, config.alarm_time, 2, 12);
    POINT_COLOR = BLUE;
	
	LCD_ShowNum(116, 66, temp, 2, 16);
	LCD_ShowNum(116, 82, rh, 2, 16);
}
