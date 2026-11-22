#include "stdio.h"
unsigned short period,duty_cycle;
float max_delay_ms = 7.0;
#define DUTY_MAX 100
#define FOSC				(16000000)
#define INT_TIME0			10	//定时时间，单位为us

#define	TH_VAL0				(unsigned char)((0x10000 - (INT_TIME0*(FOSC/1000))/12000)>>8)
#define	TL_VAL0				(unsigned char)((0x10000 - (INT_TIME0*(FOSC/1000))/12000))
int main(int argc, char const *argv[])
{
    for (int i = 0; i <= DUTY_MAX; i+=10)
    {
        duty_cycle = i;
        period = (unsigned int)(((DUTY_MAX - duty_cycle) * (max_delay_ms)) / DUTY_MAX * 100);
        printf("%d-%d\n",duty_cycle,period);
        
    }
    printf("TH_VAL0 = %02X",TH_VAL0);
    printf("TL_VAL0 = %02X",TL_VAL0);
    return 0;
}
