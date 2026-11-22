#ifndef _INTERRUPT_C_
#define _INTERRUPT_C_

/*********************************************************************************************************************/
#include "headfile.h"
/*********************************************************************************************************************/
extern void external_int4_ISR(void);
extern void Timer1s_ISR(void);
extern void Timer10us_ISR(void);
void INT2_ISR (void) interrupt 4 
{
#ifdef UART1_EN
	UART1_ISR();
#endif
}

void INT4_ISR(void) interrupt 6 
{
#ifdef UART2_EN
	UART2_ISR();
#endif
	if(EPIF & 0x04)					
	{
		EPIF = 0x04;				
		external_int4_ISR();
		
	}
}

void TIMER0_ISR (void) interrupt 1 	//每10us产生中断
{
	TH0 = TH_VAL0;
	TL0 = TL_VAL0;
	Timer10us_ISR();
}
void TIMER1_ISR (void) interrupt 3 	//每1ms产生中断
{
	TH1 = TH_VAL1;
	TL1 = TL_VAL1;
	Timer1s_ISR();
	led_indicator_task();
	
}
/*********************************************************************************************************************/
#endif
