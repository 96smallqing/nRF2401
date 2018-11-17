#include "delay.h"

void Delay1ms_8266(unsigned char t)		//@22.1184MHz  ÿ2ms
{
	unsigned char i,j,z;
	for(z=t;z>0;z--)
	{
		
		_nop_();
		_nop_();
		i = 22;
		j = 128;
		do
		{
			while (--j);
		} while (--i);
	}	
}



void Delay10us_8266()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	i = 25;
	while (--i);
}


void Delay1000ms_8266()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 43;
	j = 6;
	k = 203;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void Delay_8266(int cnt)
{
		while(cnt>0)
		{
			Delay1000ms_8266();
			cnt--;
		}
}












