#ifndef  INT_H
#define  INT_H
#include "headfile.h"


enum 
{
	P10_INDEX = 0,
	P11_INDEX = 1,
	P12_INDEX = 2,
	P13_INDEX = 3,
	P14_INDEX = 4,
	P15_INDEX = 5,	
	P04_INDEX = 6,
	P05_INDEX = 7,
	P06_INDEX = 8,
	P07_INDEX = 9,
};
#define EPIE(n)			(n<<7)
#define EPPL(n)			(n<<5)
#define EPPSEL(n)		(n<<0)

void INT4_Init(void);
#endif
