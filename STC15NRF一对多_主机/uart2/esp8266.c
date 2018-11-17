#include "esp8266.h"  //STC15W4K����11.0592MHz
#include "string.h"
#include "WIFI.h"
#include<STC15W4K.h>
/*************	���س�������	**************/
#define MAIN_Fosc		11059200L	//������ʱ��
#define	RX2_Length	4		/* ���ջ��峤�� */
#define	UART_BaudRate2	115200UL	 /* ������ */
//unsigned char config[]={"AT+MCONFIG=\"869300031657095\",\"5qx9dzUzHMvls4e9kMAO0010066c00\",\"355d19eef2104fb5bb28f8caea0b8b3a\"\x00D\x00A"};
//-------------����Ϊ���ܶ���------------------------------------------------------------
//BYTE code  WiFi_AT[]={"AT\x00D\x00A"};	                					//��ʽ�����
//BYTE code  WiFi_CWMODE[]={"AT+CWMODE=1\x00D\x00A"};	          					//��ȡ�汾��Ϣ
//BYTE code  WiFi_CWJAP[]={"AT+CWJAP_DEF=\"surfer_413\",\"surfer_413\"\x00D\x00A"};	           				//1���������磻0δ��������
//BYTE code  WiFi_CIFSR[]={"AT+CIFSR\x00D\x00A"};	//AT+SAPBR��IP���ã�CONTYPE���������ӣ�GPRS��gprs����
//BYTE code  WiFi_CIPSTART[]={"AT+CIPSTART=\"TCP\",\"120.77.58.34\",8700\x00D\x00A"};	//AT+SAPBR��IP���ã�CONTYPE���������ӣ�GPRS��gprs����
//BYTE code  WiFi_CIPMODE[]={"AT+CIPMODE=1\x00D\x00A"};	//AT+SAPBR��IP���ã�CONTYPE���������ӣ�GPRS��gprs����
//BYTE code  WiFi_CIPSEND[]={"AT+CIPSEND\x00D\x00A"};               				 //air202����HEX MQTTģʽ
//BYTE code  WiFi_CONNECT[]={"{\"t\": 1,\"device\": \"suo001\",\"key\":\"a553fd6306fd4898a7445013e28aa89a\",\"ver\":\"v1.1\"}"};               				 //air202����HEX MQTTģʽ
//BYTE    WiFi_UPDATA[]={"{\"t\": 3,\"datatype\":1,\"datas\":{\"temp\":\"0\",\"shidu\":\"46\"},\"msgid\": 123}"}; //temp��������ϴ���Ϣ

u8	xdata	RX2_Buffer[RX2_Length];	//���ջ���
u8	TX2_read,RX2_write;	//��д����(ָ��).
bit B_TX2_Busy;

//u8 flag_xintiao = 0;
void	SetTimer2Baudraye(u16 dat)	// ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
{
	AUXR &= ~(1<<4);	//Timer stop
	AUXR &= ~(1<<3);	//Timer2 set As Timer
	AUXR |=  (1<<2);	//Timer2 set as 1T mode
	TH2 = dat / 256;
	TL2 = dat % 256;
	IE2  &= ~(1<<2);	//��ֹ�ж�
	AUXR |=  (1<<4);	//Timer run enable
}
void	UART2_config(u8 brt)	// ѡ������, 2: ʹ��Timer2��������, ����ֵ: ��Ч.
{
	u8	i;
	/*********** �����ʹ̶�ʹ�ö�ʱ��2 *****************/
	if(brt == 2)	SetTimer2Baudraye(65536UL - (MAIN_Fosc / 4) / UART_BaudRate2);

	S2CON &= ~(1<<7);	// 8λ����, 1λ��ʼλ, 1λֹͣλ, ��У��
	IE2   |= 1;			//�����ж�
	S2CON |= (1<<4);	//�������
	P_SW2 &= ~1;		//�л��� P1.0 P1.1
//	P_SW2 |= 1;			//�л��� P4.6 P4.7

	for(i=0; i<RX2_Length; i++)		RX2_Buffer[i] = 0;
	B_TX2_Busy  = 0;
	TX2_read    = 0;
	RX2_write   = 0;
}
void PrintString2(u8 *puts)
{
    for (; *puts != 0;	puts++)
	{
		B_TX2_Busy = 1;		//��־����æ
		S2BUF = *puts;		//��һ���ֽ�
		while(B_TX2_Busy);	//�ȴ��������
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
			RX2_Buffer[i] = 0;//������ջ���
			//ע�⸴λһ��������������������
			TX2_read=0;
			RX2_write=0;	//��д����(ָ��).
      B_TX2_Busy=0;
		
		}
}