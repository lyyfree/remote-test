#ifndef  HEADFILE_H
#define  HEADFILE_H
#include "ca51f1s3_config.h"		
#include "ca51f1s3sfr.h"
#include "ca51f1s3xsfr.h"
#include "gpiodef_f1s3.h"
#include "system_clock.h"
#include "wifi.h"

#include "uart.h"
#include "delay.h"
#include "timer.h"
#include "int.h"
#include "key.h"

#define BT_ON 			(1)
#define BT_OFF 			(!BT_ON)
#define LED_ON			(1)
#define LED_OFF			(!LED_ON)

#define LIN 			P04			//过0检测
#define LIN_CFG			P04F
#define KEY_KG			P05			//开关按键/WiFi重置按键
#define KEY_KG_CFG		P05F		
#define KEY_LD			P03			//调亮度按键
#define KEY_LD_CFG		P03F		
#define KEY_TIM			P02			//调时间按键
#define KEY_TIM_CFG		P02F		
#define LED_WIFI		P06
#define LED_WIFI_CFG	P06F
#define LED_DY			P07			//电源灯
#define LED_DY_CFG		P07F
#define LED_2H			P12
#define LED_2H_CFG		P12F
#define LED_4H			P13
#define LED_4H_CFG		P13F
#define LED_6H			P14
#define LED_6H_CFG		P14F
#define BT				P15
#define BT_CFG			P15F
#define DUTY_MAX 100
typedef enum{
	emLightState_Off		 = 0,
	emLightState_On		 = 1,
}emLightStateTdf;

typedef enum{
    emTime_2H       = 0,
    emTime_4H       = 1,
    emTime_6H       = 2,
    // emTime_8H       = 3,
}emTimeTdf;

typedef enum{
    emTimeState_Off         = 0,
    emTimeState_On         = 1,
}emTimeStateTdf;
extern volatile emLightStateTdf xdata emLightState;
extern volatile emTimeTdf xdata  emTime;
extern volatile emTimeStateTdf xdata  emTimeState;
extern volatile u16 xdata duty_cycle_from_wifi;
extern volatile u16 xdata duty_cycle_in_mcu;
extern void SetCloseTime(void);
extern void led_indicator_task(void);
#endif
