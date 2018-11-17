#include "esp8266.h"  //STC15W4K晶振11.0592MHz
#include "string.h"
#include "WIFI.h"
#include<STC15W4K.h>
/*************	本地常量声明	**************/
#define MAIN_Fosc		11059200L	//定义主时钟
#define	RX2_Length	4		/* 接收缓冲长度 */
#define	UART_BaudRate2	115200UL	 /* 波特率 */
//unsigned char config[]={"AT+MCONFIG=\"869300031657095\",\"5qx9dzUzHMvls4e9kMAO0010066c00\",\"355d19eef2104fb5bb28f8caea0b8b3a\"\x00D\x00A"};
//-------------以下为功能定义------------------------------------------------------------
//BYTE code  WiFi_AT[]={"AT\x00D\x00A"};	                					//格式化命令。
//BYTE code  WiFi_CWMODE[]={"AT+CWMODE=1\x00D\x00A"};	          					//获取版本信息
//BYTE code  WiFi_CWJAP[]={"AT+CWJAP_DEF=\"surfer_413\",\"surfer_413\"\x00D\x00A"};	           				//1，附着网络；0未附着网络
//BYTE code  WiFi_CIFSR[]={"AT+CIFSR\x00D\x00A"};	//AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPSTART[]={"AT+CIPSTART=\"TCP\",\"120.77.58.34\",8700\x00D\x00A"};	//AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPMODE[]={"AT+CIPMODE=1\x00D\x00A"};	//AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPSEND[]={"AT+CIPSEND\x00D\x00A"};               				 //air202启用HEX MQTT模式
//BYTE code  WiFi_CONNECT[]={"{\"t\": 1,\"device\": \"suo001\",\"key\":\"a553fd6306fd4898a7445013e28aa89a\",\"ver\":\"v1.1\"}"};               				 //air202启用HEX MQTT模式
//BYTE    WiFi_UPDATA[]={"{\"t\": 3,\"datatype\":1,\"datas\":{\"temp\":\"0\",\"shidu\":\"46\"},\"msgid\": 123}"}; //temp后面跟着上传信息

u8	xdata	RX2_Buffer[RX2_Length];	//接收缓冲
u8	TX2_read,RX2_write;	//读写索引(指针).
bit B_TX2_Busy;

//u8 flag_xintiao = 0;
void	SetTimer2Baudraye(u16 dat)	// 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
{
	AUXR &= ~(1<<4);	//Timer stop
	AUXR &= ~(1<<3);	//Timer2 set As Timer
	AUXR |=  (1<<2);	//Timer2 set as 1T mode
	TH2 = dat / 256;
	TL2 = dat % 256;
	IE2  &= ~(1<<2);	//禁止中断
	AUXR |=  (1<<4);	//Timer run enable
}
void	UART2_config(u8 brt)	// 选择波特率, 2: 使用Timer2做波特率, 其它值: 无效.
{
	u8	i;
	/*********** 波特率固定使用定时器2 *****************/
	if(brt == 2)	SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / UART_BaudRate2);

	S2CON &= ~(1<<7);	// 8位数据, 1位起始位, 1位停止位, 无校验
	IE2   |= 1;			//允许中断
	S2CON |= (1<<4);	//允许接收
	P_SW2 &= ~1;		//切换到 P1.0 P1.1
//	P_SW2 |= 1;			//切换到 P4.6 P4.7

	for(i=0; i<RX2_Length; i++)		RX2_Buffer[i] = 0;
	B_TX2_Busy  = 0;
	TX2_read    = 0;
	RX2_write   = 0;
}
void PrintString2(u8 *puts)
{
    for (; *puts != 0;	puts++)
	{
		B_TX2_Busy = 1;		//标志发送忙
		S2BUF = *puts;		//发一个字节
		while(B_TX2_Busy);	//等待发送完成
	}
}
void UART2_int (void) interrupt UART2_VECTOR
{
	if(RI2)
	{
		CLR_RI2();
		RX2_Buffer[RX2_write] = S2BUF;
		if(++RX2_write >= RX2_Length)	RX2_write = 0;

	}

	if(TI2)
	{
		CLR_TI2();
		B_TX2_Busy = 0;
	}

}

//void WIFI(void)
//{
//	//SendString("ok1");
//	//PrintString2("WiFi");
//	
//	RX2_write = 0;PrintString2(WiFi_AT);
//	Delay_8266(2);
//	SendString(RX2_Buffer);   clear();
// // SendString("ok2");

//	RX2_write = 0;PrintString2(WiFi_CWMODE);
//	Delay_8266(2);SendString(RX2_Buffer);  clear();


//	RX2_write = 0;PrintString2(WiFi_CWJAP);
//	Delay_8266(6);SendString(RX2_Buffer); clear();


//	RX2_write = 0;PrintString2(WiFi_CIFSR);
//	Delay_8266(3);SendString(RX2_Buffer);  clear();


//	RX2_write = 0;PrintString2(WiFi_CIPSTART);
//	Delay_8266(6);SendString(RX2_Buffer); clear();


//	RX2_write = 0;PrintString2(WiFi_CIPMODE);
//	Delay_8266(2);SendString(RX2_Buffer);  clear();


//	RX2_write = 0;PrintString2(WiFi_CIPSEND);
//	Delay_8266(2);SendString(RX2_Buffer);	 clear();   


//	RX2_write = 0;PrintString2(WiFi_CONNECT);
//	Delay_8266(3);SendString(RX2_Buffer); clear();

//		
//}
void clear(void)
{  
		int i;
		for(i=0; i<RX2_Length; i++)	{	
			RX2_Buffer[i] = 0;//清除接收缓存
			//注意复位一下三条，串口正常工作
			TX2_read=0;
			RX2_write=0;	//读写索引(指针).
      B_TX2_Busy=0;
		
		}
}