#include "key.h"
volatile key_t  g_key_kg = {    
KEY_IDLE,
0,
0,
0,
0
};

volatile key_t  g_key_ld = {    
KEY_IDLE,
0,
0,
0,
0
};

volatile key_t  g_key_tim = {    
KEY_IDLE,
0,
0,
0,
0
};
void key_init(void)
{
    KEY_KG_CFG = INPUT | PU_EN;
    KEY_LD_CFG = INPUT | PU_EN;
    KEY_TIM_CFG = INPUT | PU_EN;
}

void key_scan(key_t *key,u8 cur_level)
{
    /* 1. 读取当前引脚电平（低有效：0=按下，1=松开） */

    switch (key->state)
    {
    /* ------------------- 空闲状态：等待按下 ------------------- */
    case KEY_IDLE:
        if (cur_level == 0)
        { // 检测到低电平（疑似按下）
            key->state = KEY_PRESS_DEBOUNCE;
            key->debounce_cnt = 0; // 消抖计数器清零
        }
        break;

    /* ------------------- 按下消抖：过滤抖动 ------------------- */
    case KEY_PRESS_DEBOUNCE:
        if (cur_level == 0)
        {
            key->debounce_cnt++;
            if (key->debounce_cnt >= 2)
            { // 连续10ms都是低电平=真按下
                key->state = KEY_PRESS_HOLD;
                key->hold_time_ms = 0; // 开始计时按下时长
            }
        }
        else
        {
            // 10ms内变回高电平=抖动，回到空闲
            key->state = KEY_IDLE;
        }
        break;

    /* ------------------- 稳定按下：计时+判断长按 ------------------- */
    case KEY_PRESS_HOLD:
        if (cur_level == 0)
        {
            key->hold_time_ms++; // 每1ms加1
            // 按下≥5s，触发长按（不用等松手，用户体验更好）
            if (key->hold_time_ms >= 200 && !key->long_press_flag)
            {
                key->long_press_flag = 1; // 置位长按标志
                key->state = KEY_LONG_TRIGGERED;
            }
        }
        else
        {
            // 提前松开，进入松开消抖
            key->state = KEY_RELEASE_DEBOUNCE;
            key->debounce_cnt = 0;
        }
        break;

    /* ------------------- 长按已触发：等松手，防重复触发 ------------------- */
    case KEY_LONG_TRIGGERED:
        if (cur_level == 1)
        {                          // 松开了
            key->state = KEY_IDLE; // 回到空闲，等待下一次按键
        }
        break;

    /* ------------------- 松开消抖：判断短按 ------------------- */
    case KEY_RELEASE_DEBOUNCE:
        if (cur_level == 1)
        {
            key->debounce_cnt++;
            if (key->debounce_cnt >= 2)
            { // 连续10ms都是高电平=真松开
                // 松开时，若未触发过长按，且按下时长在30ms~1000ms之间=短按
                if (!key->long_press_flag &&
                    key->hold_time_ms >= 3 &&
                    key->hold_time_ms < 100)
                {
                    key->short_press_flag = 1; // 置位短按标志
                }
                key->state = KEY_IDLE; // 回到空闲
            }
        }
        else
        {
            // 10ms内又变回低电平=抖动，回到稳定按下状态
            key->state = KEY_PRESS_HOLD;
        }
        break;
    }
}

netmode_t g_netmode = NETMODE_SMART;

void key_handler(void)
{
    
    /* 处理短按：开关机模式 */
    if (g_key_kg.short_press_flag)
    {
        g_key_kg.short_press_flag = 0; // 消费标志，清零
        if(emLightState == emLightState_Off)
        {
            emLightState = emLightState_On;
        }
        else
        {
            emLightState = emLightState_Off;
        }
        SetCloseTime();
        all_data_update();
    }
    
    /* 处理长按：清网+重启模组 */
    if (g_key_kg.long_press_flag)
    {
        g_key_kg.long_press_flag = 0; // 消费标志，清零
        g_netmode = NETMODE_SMART;
        mcu_reset_wifi();  // 清配网 + 重启模组，自动进 Smart
    }

    /* 处理短按：亮度模式 */
    if (g_key_ld.short_press_flag)
    {
        g_key_ld.short_press_flag = 0; // 消费标志，清零
        
        if((duty_cycle_from_wifi + 30) > 100)
        {
            duty_cycle_from_wifi = 10;
        }
        else
        {
            duty_cycle_from_wifi += 30;
        }
        all_data_update();
    }

    /* 处理短按：定时模式 */
    if (g_key_tim.short_press_flag)
    {
        g_key_tim.short_press_flag = 0; // 消费标志，清零
        if(emTimeState == emTimeState_Off)
        {
            emTimeState = emTimeState_On;
            emTime = emTime_2H;
        }
        else
        {
            emTime++;
            if(emTime >= emTime_6H)
            {
                emTimeState = emTimeState_Off;
                emTime = emTime_2H;
            }
        }
        SetCloseTime();
        all_data_update();
    }
}
