#ifndef _Delay_C_
#define _Delay_C_
/*********************************************************************************************************************/
#include "headfile.h"
/*********************************************************************************************************************/
void Delay_50us(unsigned int n)	   
{
	unsigned char i;	
#ifdef SYSCLK_16MHZ
	n *= 2;
#endif
	while(n--)
	{
		for(i=0;i<33;i++);
	}
}
void Delay_ms(unsigned int n)
{
	while(n--)
	{
		Delay_50us(20);
	}
}
void Pow_Up_Delay(unsigned int ms)	//�ο�1--1ms@CKDIV=1;//ϵͳʱ������8MHz
{
	unsigned int i;
	while(ms--)
	{
		for(i=0;i<430;i++);
	}	
}
/*********************************************************************************************************************/
#endif