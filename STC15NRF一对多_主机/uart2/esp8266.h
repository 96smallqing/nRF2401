//���� �����ڲ��Զ�ʱ��T4���ļ�
#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "WIFI.h"
#include "delay.h"
#include <STC15W4K.h>
u8	xdata	RX2_Buffer[];	//���ջ���
void clear();
//void WIFI();
void PrintString2(u8 *puts);   //ע����������ʱ�������в���Ҫд��
void UART2_config(u8 brt);
//u8 flag_xintiao ;
#endif