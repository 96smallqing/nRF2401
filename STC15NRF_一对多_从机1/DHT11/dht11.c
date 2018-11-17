#include "dht11.h"
#include "delay.h"


unsigned char tempH,tempL,humyH,humyL,CheckData;

void DHT11_Start()
{
	DHT11_Data=1;
	Delay10us();
	DHT11_Data=0;
	Delay20ms();
	DHT11_Data=1;
	Delay10us();
	Delay10us();
	Delay10us();
}

unsigned char ReceiveDHT11_Data()
{
	unsigned char i,temp,count,data_byte;
	for(i=0;i<8;i++)
	{
		count=2;
		while((!DHT11_Data)&&(count++));	//等待50us时间的结束
		temp=0;
		Delay10us();
		Delay10us();
		Delay10us();
		Delay10us();
		if(DHT11_Data==1)
		{
			temp=1;
			count=2;
			while((DHT11_Data)&&(count++));	//等待70us时间的结束
		}
		if(count==1)break;
		data_byte<<=1;
		data_byte=data_byte|temp;
	}
	return data_byte;
}

void GetDHT11_Data()
{
	unsigned char count;
	DHT11_Start();
	if(!DHT11_Data)
	{
		count=2;
		while((!DHT11_Data)&&(count++));	//等待80us低电平时间的结束
		count=2;
		while((DHT11_Data)&&(count++));	//等待80us高电平时间的结束
		humyH=ReceiveDHT11_Data();
		humyL=ReceiveDHT11_Data();
		tempH=ReceiveDHT11_Data();
		tempL=ReceiveDHT11_Data();
		CheckData=ReceiveDHT11_Data();
		DHT11_Data=1;
	}
}





