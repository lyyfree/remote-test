#include "int.h"
void INT4_Init(void)
{
	LIN_CFG = INPUT;						
	EP2CON = EPIE(1) | EPPL(2) | EPPSEL(P04_INDEX); 
	PX4 = 1;
	INT4EN = 1;						
}