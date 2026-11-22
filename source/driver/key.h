#ifndef  KEY_H
#define  KEY_H
#include "headfile.h"
/* 按键状态枚举 */
typedef enum {
    KEY_IDLE = 0,               // 空闲（未按）
    KEY_PRESS_DEBOUNCE,         // 按下消抖中
    KEY_PRESS_HOLD,             // 稳定按下状态
    KEY_LONG_TRIGGERED,         // 长按已触发（防重复）
    KEY_RELEASE_DEBOUNCE        // 松开消抖中
} key_state_t;

/* 按键实例结构体 */
typedef struct {
    key_state_t state;          // 当前状态
    u16 debounce_cnt;      // 消抖计数器（1ms加1）
    u32 hold_time_ms;      // 按下持续时间（ms）
    u8 short_press_flag;   // 短按触发标志（主循环消费后清零）
    u8 long_press_flag;    // 长按触发标志（主循环消费后清零）
} key_t;
typedef enum {
    NETMODE_SMART = 0,
    NETMODE_AP,
} netmode_t;
extern volatile key_t  g_key_kg ;
extern volatile key_t  g_key_ld ;
extern volatile key_t  g_key_tim ;
extern netmode_t g_netmode;
void key_init(void);
void key_scan(key_t *key, u8 cur_level);
void key_handler(void);
#endif
