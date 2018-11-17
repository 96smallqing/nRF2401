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
		while((!DHT11_Data)&&(count++));	//�ȴ�50usʱ��Ľ���
		temp=0;
		Delay10us();
		Delay10us();
		Delay10us();
		Delay10us();
		if(DHT11_Data==1)
		{
			temp=1;
			count=2;
			while((DHT11_Data)&&(count++));	//�ȴ�70usʱ��Ľ���
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
		while((!DHT11_Data)&&(count++));	//�ȴ�80us�͵�ƽʱ��Ľ���
		count=2;
		while((DHT11_Data)&&(count++));	//�ȴ�80us�ߵ�ƽʱ��Ľ���
		humyH=ReceiveDHT11_Data();
		humyL=ReceiveDHT11_Data();
		tempH=ReceiveDHT11_Data();
		tempL=ReceiveDHT11_Data();
		CheckData=ReceiveDHT11_Data();
		DHT11_Data=1;
	}
}





