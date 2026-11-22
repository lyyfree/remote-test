#ifndef _MAIN_C_
#define _MAIN_C_
/*********************************************************************************************************************/
#include "headfile.h"
/*********************************************************************************************************************/
// typedef unsigned long u32;
// typedef unsigned int u16;
// typedef unsigned char u8;
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))









#define DAY_SECONDS 86400	  // 24小时秒数
// #define ONE_HOUS_SECONDS 10 // 小时秒数
#define ONE_HOUS_SECONDS 3600 // 小时秒数

// 变量声明
volatile emLightStateTdf xdata emLightState = 0;
volatile emTimeTdf xdata emTime = 0;
volatile emTimeStateTdf xdata emTimeState = 0;

// 变量声明(全局且需volatile)
volatile unsigned int xdata cycleCounter = 0;	// 周期计数器
volatile unsigned int xdata period = 0;			// 初始周期值=10ms（10000 * 10us）
volatile bit counterRunning = 0;				// 计数器运行标志
volatile bit inPulsePhase = 0;					// 1ms脉冲阶段标志
volatile unsigned int xdata msPulseCounter = 0; // 1ms脉冲计数器

volatile bit day_flag; // 记录过了一天

volatile u32 xdata sys_time = 0; // 系统时间（秒）
volatile bit is_cycle = 0;		 // 循环模式标志
u32 xdata turn_on_time = 0;		 // 开灯时间戳
u32 xdata turn_off_time = 0;	 // 关灯时间戳

// 全局变量定义
volatile unsigned int  timerCount = 0; // 1ms定时器计数器
volatile unsigned char xdata edgeCount = 0; // 边沿计数
volatile bit detectionDone = 0;				// 检测完成标志
volatile float xdata max_delay_ms = 0;		// 结果存储变量
volatile u8 xdata hours = 0;
volatile unsigned int  Total_value = 0;
volatile u16 xdata ms_tick;
volatile u16 xdata duty_cycle_from_wifi = 100; // wifi传过来的百分比
volatile u16 xdata duty_cycle_in_mcu = 0;	   // 对wifi传过来的百分比处理后的数据

// LIN引脚中断服务程序（INT1）
void LIN_Int()
{
	// 当收集到4个边沿（3个完整周期）时进行计算
	if (timerCount > 300)
	{
		char current_is_60Hz = 0;
		char current_is_50Hz = 0;
		static unsigned char detection_attempts = 0; // 检测尝试次数
		static unsigned char valid_count_60Hz = 0;	 // 符合60Hz的有效计数
		static unsigned char valid_count_50Hz = 0;	 // 符合
													 // 计算周期（取三个周期的值）
		Total_value = timerCount;

		detection_attempts++;
		// 检查当前检测值是否符合频率范围
		current_is_60Hz = (Total_value >= 600 && Total_value < 800);
		current_is_50Hz = (Total_value >= 800 && Total_value <= 1200);
		// current_is_60Hz = (Total_value >= 600 && Total_value < 900);
		// current_is_50Hz = (Total_value >= 900 && Total_value <= 1200);
		duty_cycle_from_wifi = Total_value / 10;
		if (current_is_60Hz)
		{
			valid_count_60Hz++;
		}
		else if (current_is_50Hz)
		{
			valid_count_50Hz++;
		}
		// 如果已经检测了3次
		if (detection_attempts >= 3)
		{
			// 检查是否连续3次都符合同一个频率
			if (valid_count_60Hz == 3)
			{
				// 3次都符合60Hz
				max_delay_ms = 7.8f;
				TR0 = 0;		   // 关闭Timer0 10us
				detectionDone = 1; // 标记检测完成
			}
			else if (valid_count_50Hz == 3)
			{
				// 3次都符合50Hz
				max_delay_ms = 10.0f;
				TR0 = 0;		   // 关闭Timer0 10us
				detectionDone = 1; // 标记检测完成
			}
			else
			{
				// 检测失败，重置状态重新检测
				// 可以选择保留部分历史数据或者完全重置
				detection_attempts = 1; // 保留最近一次的数据
				valid_count_60Hz = (current_is_60Hz ? 1 : 0);
				valid_count_50Hz = (current_is_50Hz ? 1 : 0);
				timerCount = 0;
			}
		}
		else
		{

			timerCount = 0;
		}
	}
}
// 启动周期计数器
void StartCycleCounter(void)
{
	counterRunning = 1;
	TH0 = TH_VAL0;	  // 高8位装初值
	TL0 = TL_VAL0;	  // 低8位装初值
	TR0 = 1;		  // 使能Timer0 10us
	BT = BT_OFF; // 确保启动时输出为高
}

void Timer10us_ISR(void)
{
	if (detectionDone == 0)
	{				  // 检测频率
		timerCount++; // 全局计时器递增
		return;
	}
	// 1ms脉冲处理逻辑
	if (inPulsePhase)
	{
		if (--msPulseCounter == 0)
		{
			inPulsePhase = 0; // 结束脉冲阶段
			TR0 = 0;		  // 关闭Timer0 10us
			BT = BT_OFF; // 恢复高电平
		}
	}

	// 周期计数器逻辑
	if (counterRunning)
	{
		if (++cycleCounter >= period + 10)
		{
			counterRunning = 0;
			cycleCounter = 0;
			inPulsePhase = 1;
			msPulseCounter = 59; // 100×10us = 1ms
			BT = BT_ON;		 // 输出低电平
		}
	}
}

void external_int4_ISR(void)
{
	if (detectionDone == 0)
	{ // 检测频率
		if (edgeCount == 3)
		{
			TR0 = 1; // 使能Timer0 10us
			LIN_Int();
		}
		else
		{
			edgeCount++;
		}
	}
	else
	{
		if (emLightState == emLightState_On)
		{
			// 达到90%全部拉低BT
			StartCycleCounter(); // 启动
		}
	}
}

void Timer1s_ISR(void)
{
	ms_tick++;
	if(ms_tick % 10 == 0)
	{
		key_scan(&g_key_kg,KEY_KG);
		key_scan(&g_key_ld,KEY_LD);
		key_scan(&g_key_tim,KEY_TIM);
	}
	if (ms_tick >= 1000)
	{
		ms_tick = 0;
		// 系统时钟（每日归零）
		sys_time = (sys_time < DAY_SECONDS - 1) ? (sys_time + 1) : 0;
		if (!sys_time)
			day_flag = 1;
	}
}

void SetCloseTime(void)
{
	if (emTimeState == emTimeState_On)
	{
		day_flag = 0;
		turn_on_time = sys_time; // 记录开灯时刻
		switch (emTime)
		{
		case emTime_2H:
			hours = 2;
			break;
		case emTime_4H:
			hours = 4;
			break;
		case emTime_6H:
			hours = 6;
			break;
		}
		turn_off_time = turn_on_time + (u16)hours * ONE_HOUS_SECONDS;
	}
}

/**
 * 查找表：功率(0.1W) → mcu_duty
 * 数据来源：你的实测结果，按功率从小到大排列
 * 注：mcu_duty越大，导通角越小，功率越低
 */
typedef struct
{
	u16 p_01w;	 // 功率，单位0.1W（100=10.0W，450=45.0W）
	u8 mcu_duty; // MCU内部占空比（0~100，≤58时功率≥10W，稳定）
} P2Duty_LUT_t;

P2Duty_LUT_t xdata p2duty_lut[] = {
	{100, 60}, // 10.0W → 稳定下限，对应d_wifi=1时的目标功率
	{136, 55}, // 13.6W
	{171, 50}, // 17.1W
	{230, 45}, // 23.0W
	{275, 40}, // 27.5W
	{308, 35}, // 30.8W
	{342, 30}, // 34.2W
	{376, 25}, // 37.6W
	{397, 20}, // 39.7W
	{410, 15}, // 41.0W
	{436, 10}, // 43.6W
	{480, 0},  // 48.0W → 目标上限，实测全通时为48W，此处对齐目标
};
#define LUT_SIZE (sizeof(p2duty_lut) / sizeof(p2duty_lut[0]))

/**
 * @brief 根据WiFi占空比计算目标功率（线性映射）
 * @param d_wifi: 来自WiFi的占空比（1~100）
 * @return 目标功率，单位0.1W（100~480）
 */
static u16 calc_target_power(u8 d_wifi)
{
	u32 p_target_temp = 0;
	// 输入健壮性处理（无限制，仅防止极端值）
	if (d_wifi < 1)
		d_wifi = 1;
	if (d_wifi > 100)
		d_wifi = 100;

	// P_target = 10W + (48W-10W)*(d_wifi-1)/(100-1)
	// 放大10倍转整数运算：100 + 350*(d_wifi-1)/99
	p_target_temp = 100 + (380 * (d_wifi - 1)) / 99;

	// 硬件上限钳位：实测最大功率为48W，无法突破48W，此处对齐目标
	if (p_target_temp > 480)
		p_target_temp = 480;

	return (u16)p_target_temp;
}

/**
 * @brief 根据目标功率反查MCU占空比（基于实测数据插值）
 * @param target_p_01w: 目标功率，单位0.1W
 * @return MCU内部占空比（0~58，保证功率≥10W，稳定）
 */
static u8 get_mcu_duty_by_power(u16 target_p_01w)
{
	u8 i;
	u8 d_low, d_high;
	u16 p_low, p_high;
	u32 mcu_duty = 0;
	// 边界1：功率≤10W，返回最大稳定占空比58
	if (target_p_01w <= p2duty_lut[0].p_01w)
	{
		return p2duty_lut[0].mcu_duty;
	}
	// 边界2：功率≥48W，返回最小占空比0（全通）
	if (target_p_01w >= p2duty_lut[LUT_SIZE - 1].p_01w)
	{
		return p2duty_lut[LUT_SIZE - 1].mcu_duty;
	}

	// 查找功率所在的插值区间（按功率升序排列）
	for (i = 0; i < LUT_SIZE - 1; i++)
	{
		if (target_p_01w >= p2duty_lut[i].p_01w &&
			target_p_01w <= p2duty_lut[i + 1].p_01w)
		{
			p_low = p2duty_lut[i].p_01w;
			p_high = p2duty_lut[i + 1].p_01w;
			d_low = p2duty_lut[i].mcu_duty;
			d_high = p2duty_lut[i + 1].mcu_duty;
			break;
		}
	}

	/**
	 * 线性插值（功率↑ → mcu_duty↓，反向关系）
	 * 公式：d = d_low - (target_p - p_low) * (d_low - d_high) / (p_high - p_low)
	 * 例：target_p=167（16.7W），介于171(17.1W, d=50)和136(13.6W, d=55)之间
	 * d=50 - (167-136)*(50-55)/(171-136) = 50 - 31*(-5)/35 ≈ 54.4
	 */
	mcu_duty = d_low;
	if (p_high != p_low)
	{ // 避免除零
		mcu_duty -= (u32)(target_p_01w - p_low) * (d_low - d_high) / (p_high - p_low);
	}

	// 最终钳位：最大占空比不超过60（保证功率≥10W，稳定）
	if (mcu_duty > p2duty_lut[0].mcu_duty)
	{
		mcu_duty = p2duty_lut[0].mcu_duty;
	}
	// 最小占空比不低于0
	if (mcu_duty > 100)
		mcu_duty = 100; // 溢出保护

	return (u8)mcu_duty;
}

volatile u32 xdata blink_cnt = 0;
void led_indicator_task(void)
{
	blink_cnt++;
	switch (emLightState)
	{
	case emLightState_Off:
		{
			LED_DY = LED_OFF;
			LED_2H = LED_OFF;
			LED_4H = LED_OFF;
			LED_6H = LED_OFF;
			break;
		}
	case emLightState_On:
		{
			LED_DY = LED_ON;
			if(emTimeState == emTimeState_Off)
			{
				LED_2H = LED_OFF;
				LED_4H = LED_OFF;
				LED_6H = LED_OFF;
			}
			else
			{
				switch (emTime)
				{

					case emTime_2H:
					{
						LED_2H = LED_ON;
						LED_4H = LED_OFF;
						LED_6H = LED_OFF;
						break;
					}
					case emTime_4H:
					{
						LED_2H = LED_OFF;
						LED_4H = LED_ON;
						LED_6H = LED_OFF;
						break;
					}
					case emTime_6H:
					// case emTime_8H:
					{
						LED_2H = LED_OFF;
						LED_4H = LED_OFF;
						LED_6H = LED_ON;
						break;
					}
				}
			}
			break;
		}
	}
	switch (mcu_get_wifi_work_state())
	{
		case SMART_CONFIG_STATE: // 0x00
								// 快闪 250ms = 亮125 + 灭125（假设blink_cnt是1ms计）
			if ((blink_cnt & 0x7F) < 0x40)
			{
				LED_WIFI = LED_ON;
			}
			else
			{
				LED_WIFI = LED_OFF;
			}
			break;
		case AP_STATE: // 0x01
					// 慢闪 1500ms = 亮750 + 灭750
			if ((blink_cnt & 0x5FF) < 0x2FF)
			{
				LED_WIFI = LED_ON;
			}
			else
			{
				LED_WIFI = LED_OFF;
			}
			break;
		case WIFI_CONNECTED:  // 0x03
		case WIFI_CONN_CLOUD: // 0x04  ← 已连云，业务解锁
			LED_WIFI = LED_ON;	  // 常亮
			break;
		case WIFI_NOT_CONNECTED: // 0x02
		default:
			LED_WIFI = LED_OFF; // 熄
			break;
	}
}

/*********************************************************************************************************************/

void System_Init(void)
{
	LVDCON = 0xE2; // 设置LVD复位电压为2.7V
#ifdef SYSCLK_16MHZ
	CKDIV = 0; // 系统时钟运行16MHz
#else
	CKDIV = 1; // 系统时钟运行8MHz
#endif
	IHCFG = ReadIHCFG();
	TKCCFG = ReadTKCCFG();
#ifdef UART1_EN
	Uart1_Initial(UART1_BAUTRATE);
#endif
#ifdef UART2_EN
	Uart2_Initial(UART2_BAUTRATE);
#endif
	BT_CFG = OUTPUT;
	BT = BT_OFF;
	LED_WIFI_CFG = OUTPUT | PD_EN;
	LED_DY_CFG   = OUTPUT | PD_EN;
	LED_2H_CFG   = OUTPUT | PD_EN;
	LED_4H_CFG   = OUTPUT | PD_EN;
	LED_6H_CFG   = OUTPUT | PD_EN;

}

void main(void)
{
	System_Init();
	EA = 1; // 开全局中断

#ifdef PRINT_EN
	uart_printf("Timer0 Mode1 Demo Code\n");
#endif

	Timer0_Init();
	Timer1_Init();
	INT4_Init();
	key_init();
	wifi_protocol_init();
	// LED_WIFI = 1;
	while (detectionDone == 0);
	// LED_WIFI = 0;
	// delay_ms(200);
	// duty_cycle_from_wifi = 0;
	// emLightState = emLightState_Off;

	// while(1)
	// {
	// 	period = (unsigned int)((duty_cycle_from_wifi * (max_delay_ms)) / DUTY_MAX * 100);
	// 	/* 处理短按：切换Smart/AP配网模式 */
	//     if (g_key.short_press_flag) {
	//         g_key.short_press_flag = 0;  // 消费标志，清零
	// 		LED_WIFI  = !LED_WIFI;
	// 		duty_cycle_from_wifi += 10;
	// 		if(duty_cycle_from_wifi >= 70)
	// 		{
	// 			duty_cycle_from_wifi = 0;
	// 		}
	//         key_short_press_handler();   // 你之前写的切模式函数
	//     }

	//     /* 处理长按：清网+重启模组 */
	//     if (g_key.long_press_flag) {
	//         g_key.long_press_flag = 0;   // 消费标志，清零
	// 		emLightState = !emLightState;
	//         key_long_press_handler();    // 你之前写的清网函数
	//     }
	// }
	while (1)
	{
		wifi_uart_service();
		key_handler();
		duty_cycle_in_mcu = get_mcu_duty_by_power(calc_target_power(duty_cycle_from_wifi));
		period = (unsigned int)((duty_cycle_in_mcu * (max_delay_ms)) / DUTY_MAX * 100);

		// 自动关灯检测
		if (emLightState == emLightState_On && emTimeState == emTimeState_On)
		{
			if (sys_time >= turn_off_time ||
				(turn_off_time > DAY_SECONDS && sys_time >= (turn_off_time % DAY_SECONDS) && day_flag))
			{
				emLightState = emLightState_Off;
				emTimeState = emTimeState_Off;
				turn_on_time = 0;
				turn_off_time = 0;
				if (day_flag == 1)
				{
					day_flag = 0;
				}
				all_data_update();
			}
		}

	}
}
#endif
